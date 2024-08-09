#include <gtk/gtk.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include "fmradio.h"

// keep it in a struct maybe
static int fd;
static guint timeout_id = 0;
static GtkWidget *frequency_display;
static GtkWidget *frequency_entry;
static GtkWidget *start_button;
static GtkWidget *stop_button;
static GtkWidget *output_text_view;
static GtkTextBuffer *output_buffer;
static GtkWidget *volume_scale;
static GtkWidget *tune_up_button;
static GtkWidget *tune_down_button;
static GtkWidget *seek_up_button;
static GtkWidget *seek_down_button;
static GtkWidget *preset_buttons[5];
static GtkWidget *mute_button;
static gboolean is_muted = FALSE;
static int current_frequency = 8750;

static void append_to_output(const char *format, ...) {
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(output_buffer, &iter);
    gtk_text_buffer_insert(output_buffer, &iter, buffer, -1);
    gtk_text_buffer_insert(output_buffer, &iter, "\n", -1);

    GtkTextMark *mark = gtk_text_buffer_get_insert(output_buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(output_text_view), mark, 0.0, TRUE, 0.0, 1.0);
}

static void update_frequency_display(float freq) {
    char freq_str[20];
    snprintf(freq_str, sizeof(freq_str), "%.1f MHz", freq);
    gtk_label_set_text(GTK_LABEL(frequency_display), freq_str);
}

static gboolean run_tests(gpointer user_data) {
    int rssi, vol;

    int ret = fm_getrssi(fd, &rssi);
    if (ret < 0)
        append_to_output("Error getting RSSI");
    else
        append_to_output("RSSI: %d", rssi);

    ret = fm_getvol(fd, &vol);
    if (ret < 0)
        append_to_output("Error getting volume");
    else
        append_to_output("Volume: %d", vol);

    return G_SOURCE_CONTINUE;
}

static void update_start_button_sensitivity(void) {
    const char *text = gtk_editable_get_text(GTK_EDITABLE(frequency_entry));
    gboolean is_empty = (text == NULL || text[0] == '\0');
    gtk_widget_set_sensitive(start_button, !is_empty);
}

static void on_frequency_entry_changed(GtkEditable *editable, gpointer user_data) {
    update_start_button_sensitivity();
}

static void on_mute_toggled(GtkToggleButton *button, gpointer user_data) {
    is_muted = gtk_toggle_button_get_active(button);
    int ret = fm_mute(fd, is_muted ? 1 : 0);
    if (ret < 0)
        append_to_output("Error setting mute state");
    else
        append_to_output("Radio %s", is_muted ? "muted" : "unmuted");

    gtk_widget_set_sensitive(volume_scale, !is_muted);
}

static void on_start_clicked(GtkButton *button, gpointer user_data) {
    const gchar *freq_str = gtk_editable_get_text(GTK_EDITABLE(frequency_entry));
    if (freq_str == NULL || freq_str[0] == '\0') {
        append_to_output("Please enter a frequency before starting");
        return;
    }

    float freq_float = atof(freq_str);
    if (freq_float < 87.5 || freq_float > 108.0) {
        append_to_output("Invalid frequency. Please enter a value between 87.5 and 108.0");
        return;
    }

    current_frequency = (int)(freq_float * 100);

    int ret = fm_open_dev(FM_DEV, &fd);
    if (ret < 0) {
        append_to_output("Error opening device");
        return;
    }

    ret = fm_powerup(fd, FM_BAND_UE, current_frequency);
    if (ret < 0) {
        append_to_output("Error powering up");
        fm_close_dev(fd);
        return;
    }

    append_to_output("FM Radio powered up");

    int initial_volume = 15;  // 15 ix max
    ret = fm_setvol(fd, initial_volume);
    if (ret < 0)
        append_to_output("Error setting initial volume");
    else
        append_to_output("Initial volume set to %d", initial_volume);

    gtk_range_set_value(GTK_RANGE(volume_scale), initial_volume);

    gtk_widget_set_sensitive(start_button, FALSE);
    gtk_widget_set_sensitive(stop_button, TRUE);
    gtk_widget_set_sensitive(tune_up_button, TRUE);
    gtk_widget_set_sensitive(tune_down_button, TRUE);
    gtk_widget_set_sensitive(seek_up_button, TRUE);
    gtk_widget_set_sensitive(seek_down_button, TRUE);
    gtk_widget_set_sensitive(volume_scale, TRUE);
    gtk_widget_set_sensitive(mute_button, TRUE);

    for (int i = 0; i < 5; i++) {
        gtk_widget_set_sensitive(preset_buttons[i], TRUE);
    }

    update_frequency_display(freq_float);

    append_to_output("Radio started at %.1f MHz", freq_float);

    ret = fm_mute(fd, 0);
    if (ret < 0)
        append_to_output("Error unmuting radio on startup");
    else
        is_muted = FALSE;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mute_button), FALSE);

    timeout_id = g_timeout_add_seconds(2, run_tests, NULL);

    struct fm_hw_info hwinfo;
    fm_get_hw_info(fd, &hwinfo);
    append_to_output("chip id: %d", hwinfo.chip_id);
    append_to_output("eco version: %d", hwinfo.eco_ver);
    append_to_output("rom version: %d", hwinfo.rom_ver);
    append_to_output("patch version: %d", hwinfo.patch_ver);
    append_to_output("reserve: %d", hwinfo.reserve);
}

static void on_stop_clicked(GtkButton *button, gpointer user_data) {
    if (timeout_id != 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }

    int ret = fm_powerdown(fd, 0);
    if (ret < 0)
        append_to_output("Error powering down");
    else
        append_to_output("FM Radio powered down");

    ret = fm_close_dev(fd);
    if (ret < 0)
        append_to_output("Error closing device");

    gtk_widget_set_sensitive(start_button, TRUE);
    gtk_widget_set_sensitive(stop_button, FALSE);
    gtk_widget_set_sensitive(tune_up_button, FALSE);
    gtk_widget_set_sensitive(tune_down_button, FALSE);
    gtk_widget_set_sensitive(volume_scale, FALSE);
    gtk_widget_set_sensitive(mute_button, FALSE);

    for (int i = 0; i < 5; i++) {
        gtk_widget_set_sensitive(preset_buttons[i], FALSE);
    }
}

static void on_volume_changed(GtkRange *range, gpointer user_data) {
    int volume = (int)gtk_range_get_value(range);
    fm_setvol(fd, volume);
}

static void on_tune_clicked(GtkButton *button, gpointer user_data) {
    const gchar *freq_str = gtk_editable_get_text(GTK_EDITABLE(frequency_entry));
    float freq = atof(freq_str);

    if (button == GTK_BUTTON(tune_up_button))
        freq += 0.1;
    else
        freq -= 0.1;

    if (freq < 87.5)
        freq = 87.5;
    if (freq > 108.0)
        freq = 108.0;

    char new_freq_str[10];
    snprintf(new_freq_str, sizeof(new_freq_str), "%.1f", freq);
    gtk_editable_set_text(GTK_EDITABLE(frequency_entry), new_freq_str);

    current_frequency = (int)(freq * 100);
    int ret = fm_tune(fd, current_frequency, FM_BAND_UE); // maybe make configurable in a settings page
    if (ret < 0) {
        append_to_output("Error tuning to new frequency");
    } else {
        update_frequency_display(freq);
        append_to_output("Tuned to %.1f MHz", freq);
    }
}

static void on_preset_clicked(GtkButton *button, gpointer user_data) {
    int preset_number = GPOINTER_TO_INT(user_data);
    append_to_output("Preset %d clicked", preset_number);
    // make it configurable i guess? my dads car had the 1 2 3 4 5 buttons and they were configurable
}

static void on_seek_clicked(GtkButton *button, gpointer user_data) {
    int direction = GPOINTER_TO_INT(user_data);
    int ret;
    int freq = current_frequency;
    float freq_formatted;

    // 1 for up, 0 for down
    ret = fm_seek(fd, &freq, FM_BAND_UE, direction, FM_SEEKTH_LEVEL_DEFAULT);

    if (ret < 0) {
        append_to_output("Error seeking to new frequency");
        return;
    }

    printf("freq: %d", freq);
    current_frequency = freq;
    freq_formatted = freq / 100.0;

    char freq_str[10];
    snprintf(freq_str, sizeof(freq_str), "%.1f", freq_formatted);
    gtk_editable_set_text(GTK_EDITABLE(frequency_entry), freq_str);

    update_frequency_display(freq_formatted);
    append_to_output("Seeked to %.1f MHz", freq_formatted);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkBuilder *builder;
    GtkWidget *window;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "fmradio.ui", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtk_window_set_application(GTK_WINDOW(window), app);

    frequency_display = GTK_WIDGET(gtk_builder_get_object(builder, "frequency_display"));
    frequency_entry = GTK_WIDGET(gtk_builder_get_object(builder, "frequency_entry"));
    start_button = GTK_WIDGET(gtk_builder_get_object(builder, "start_button"));
    stop_button = GTK_WIDGET(gtk_builder_get_object(builder, "stop_button"));
    output_text_view = GTK_WIDGET(gtk_builder_get_object(builder, "output_text_view"));
    volume_scale = GTK_WIDGET(gtk_builder_get_object(builder, "volume_scale"));
    tune_up_button = GTK_WIDGET(gtk_builder_get_object(builder, "tune_up_button"));
    tune_down_button = GTK_WIDGET(gtk_builder_get_object(builder, "tune_down_button"));
    seek_up_button = GTK_WIDGET(gtk_builder_get_object(builder, "seek_up_button"));
    seek_down_button = GTK_WIDGET(gtk_builder_get_object(builder, "seek_down_button"));
    mute_button = GTK_WIDGET(gtk_builder_get_object(builder, "mute_button"));

    output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_text_view));

    g_signal_connect(frequency_entry, "changed", G_CALLBACK(on_frequency_entry_changed), NULL);
    g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_clicked), NULL);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop_clicked), NULL);
    g_signal_connect(volume_scale, "value-changed", G_CALLBACK(on_volume_changed), NULL);
    g_signal_connect(tune_up_button, "clicked", G_CALLBACK(on_tune_clicked), NULL);
    g_signal_connect(tune_down_button, "clicked", G_CALLBACK(on_tune_clicked), NULL);
    g_signal_connect(seek_up_button, "clicked", G_CALLBACK(on_seek_clicked), GINT_TO_POINTER(1));
    g_signal_connect(seek_down_button, "clicked", G_CALLBACK(on_seek_clicked), GINT_TO_POINTER(0));
    g_signal_connect(mute_button, "toggled", G_CALLBACK(on_mute_toggled), NULL);

    for (int i = 0; i < 5; i++) {
        char button_name[20];
        snprintf(button_name, sizeof(button_name), "preset_%d", i+1);
        preset_buttons[i] = GTK_WIDGET(gtk_builder_get_object(builder, button_name));
        g_signal_connect(preset_buttons[i], "clicked", G_CALLBACK(on_preset_clicked), GINT_TO_POINTER(i+1));
    }

    gtk_widget_set_sensitive(tune_up_button, FALSE);
    gtk_widget_set_sensitive(tune_down_button, FALSE);
    gtk_widget_set_sensitive(seek_up_button, FALSE);
    gtk_widget_set_sensitive(seek_down_button, FALSE);
    gtk_widget_set_sensitive(volume_scale, FALSE);
    gtk_widget_set_sensitive(mute_button, FALSE);

    for (int i = 0; i < 5; i++) {
        gtk_widget_set_sensitive(preset_buttons[i], FALSE);
    }

    update_start_button_sensitivity();

    g_object_unref(builder);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("io.FuriOS.FMRadio", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
