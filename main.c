/*
 * Copyright (C) 2024 Bardia Moshiri
 * SPDX-License-Identifier: GPL-3.0+
 * Author: Bardia Moshiri <bardia@furilabs.com>
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include "fmradio.h"

typedef struct {
    int fd;
    int current_frequency;
    gboolean is_muted;
    guint timeout_id;

    GtkWidget *frequency_display;
    GtkWidget *frequency_entry;
    GtkWidget *start_button;
    GtkWidget *stop_button;
    GtkWidget *output_text_view;
    GtkTextBuffer *output_buffer;
    GtkWidget *volume_scale;
    GtkWidget *tune_up_button;
    GtkWidget *tune_down_button;
    GtkWidget *seek_up_button;
    GtkWidget *seek_down_button;
    GtkWidget *preset_buttons[5];
    GtkWidget *mute_button;
} FMRadioApp;

static void append_to_output(FMRadioApp *app, const char *format, ...) {
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app->output_buffer, &iter);
    gtk_text_buffer_insert(app->output_buffer, &iter, buffer, -1);
    gtk_text_buffer_insert(app->output_buffer, &iter, "\n", -1);

    GtkTextMark *mark = gtk_text_buffer_get_insert(app->output_buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app->output_text_view), mark, 0.0, TRUE, 0.0, 1.0);
}

static void update_frequency_display(FMRadioApp *app, float freq) {
    char freq_str[20];
    snprintf(freq_str, sizeof(freq_str), "%.1f MHz", freq);
    gtk_label_set_text(GTK_LABEL(app->frequency_display), freq_str);
}

static gboolean run_tests(gpointer user_data) {
    FMRadioApp *app = (FMRadioApp *)user_data;
    int rssi, vol;

    int ret = fm_getrssi(app->fd, &rssi);
    if (ret < 0)
        append_to_output(app, "Error getting RSSI");
    else
        append_to_output(app, "RSSI: %d", rssi);

    ret = fm_getvol(app->fd, &vol);
    if (ret < 0)
        append_to_output(app, "Error getting volume");
    else
        append_to_output(app, "Volume: %d", vol);

    return G_SOURCE_CONTINUE;
}

static void handle_start_sensitivity(FMRadioApp *app) {
    const char *text = gtk_editable_get_text(GTK_EDITABLE(app->frequency_entry));
    gboolean is_empty = (text == NULL || text[0] == '\0');
    gtk_widget_set_sensitive(app->start_button, !is_empty);
}

static void on_frequency_entry_changed(GtkEditable *editable, gpointer user_data) {
    FMRadioApp *app = (FMRadioApp *)user_data;
    handle_start_sensitivity(app);
}

static void on_mute_toggled(GtkToggleButton *button, gpointer user_data) {
    FMRadioApp *app = (FMRadioApp *)user_data;
    app->is_muted = gtk_toggle_button_get_active(button);
    int ret = fm_mute(app->fd, app->is_muted ? 1 : 0);
    if (ret < 0)
        append_to_output(app, "Error setting mute state");
    else
        append_to_output(app, "Radio %s", app->is_muted ? "muted" : "unmuted");

    gtk_widget_set_sensitive(app->volume_scale, !app->is_muted);
}

static void on_start_clicked(GtkButton *button, gpointer user_data) {
    FMRadioApp *app = (FMRadioApp *)user_data;
    const gchar *freq_str = gtk_editable_get_text(GTK_EDITABLE(app->frequency_entry));
    if (freq_str == NULL || freq_str[0] == '\0') {
        append_to_output(app, "Please enter a frequency before starting");
        return;
    }

    float freq_float = atof(freq_str);
    if (freq_float < 87.5 || freq_float > 108.0) {
        append_to_output(app, "Invalid frequency. Please enter a value between 87.5 and 108.0");
        return;
    }

    app->current_frequency = (int)(freq_float * 100);

    int ret = fm_open_dev(FM_DEV, &app->fd);
    if (ret < 0) {
        append_to_output(app, "Error opening device");
        return;
    }

    ret = fm_powerup(app->fd, FM_BAND_UE, app->current_frequency);
    if (ret < 0) {
        append_to_output(app, "Error powering up");
        fm_close_dev(app->fd);
        return;
    }

    append_to_output(app, "FM Radio powered up");

    int initial_volume = 15;  // 15 is max
    ret = fm_setvol(app->fd, initial_volume);
    if (ret < 0)
        append_to_output(app, "Error setting initial volume");
    else
        append_to_output(app, "Initial volume set to %d", initial_volume);

    gtk_range_set_value(GTK_RANGE(app->volume_scale), initial_volume);

    gtk_widget_set_sensitive(app->start_button, FALSE);
    gtk_widget_set_sensitive(app->stop_button, TRUE);
    gtk_widget_set_sensitive(app->tune_up_button, TRUE);
    gtk_widget_set_sensitive(app->tune_down_button, TRUE);
    gtk_widget_set_sensitive(app->seek_up_button, TRUE);
    gtk_widget_set_sensitive(app->seek_down_button, TRUE);
    gtk_widget_set_sensitive(app->volume_scale, TRUE);
    gtk_widget_set_sensitive(app->mute_button, TRUE);

    for (int i = 0; i < 5; i++) {
        gtk_widget_set_sensitive(app->preset_buttons[i], TRUE);
    }

    update_frequency_display(app, freq_float);
    append_to_output(app, "Radio started at %.1f MHz", freq_float);

    ret = fm_mute(app->fd, 0);
    if (ret < 0)
        append_to_output(app, "Error unmuting radio on startup");
    else
        app->is_muted = FALSE;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(app->mute_button), FALSE);

    app->timeout_id = g_timeout_add_seconds(2, run_tests, app);

    struct fm_hw_info hwinfo;
    fm_get_hw_info(app->fd, &hwinfo);
    append_to_output(app, "chip id: %d", hwinfo.chip_id);
    append_to_output(app, "eco version: %d", hwinfo.eco_ver);
    append_to_output(app, "rom version: %d", hwinfo.rom_ver);
    append_to_output(app, "patch version: %d", hwinfo.patch_ver);
    append_to_output(app, "reserve: %d", hwinfo.reserve);
}

static void on_stop_clicked(GtkButton *button, gpointer user_data) {
    FMRadioApp *app = (FMRadioApp *)user_data;
    if (app->timeout_id != 0) {
        g_source_remove(app->timeout_id);
        app->timeout_id = 0;
    }

    int ret = fm_powerdown(app->fd, 0);
    if (ret < 0)
        append_to_output(app, "Error powering down");
    else
        append_to_output(app, "FM Radio powered down");

    ret = fm_close_dev(app->fd);
    if (ret < 0)
        append_to_output(app, "Error closing device");

    gtk_widget_set_sensitive(app->start_button, TRUE);
    gtk_widget_set_sensitive(app->stop_button, FALSE);
    gtk_widget_set_sensitive(app->tune_up_button, FALSE);
    gtk_widget_set_sensitive(app->tune_down_button, FALSE);
    gtk_widget_set_sensitive(app->volume_scale, FALSE);
    gtk_widget_set_sensitive(app->mute_button, FALSE);

    for (int i = 0; i < 5; i++) {
        gtk_widget_set_sensitive(app->preset_buttons[i], FALSE);
    }
}

static void on_volume_changed(GtkRange *range, gpointer user_data) {
    FMRadioApp *app = (FMRadioApp *)user_data;
    int volume = (int)gtk_range_get_value(range);
    fm_setvol(app->fd, volume);
}

static void on_tune_clicked(GtkButton *button, gpointer user_data) {
    FMRadioApp *app = (FMRadioApp *)user_data;
    const gchar *freq_str = gtk_editable_get_text(GTK_EDITABLE(app->frequency_entry));
    float freq = atof(freq_str);

    if (button == GTK_BUTTON(app->tune_up_button))
        freq += 0.1;
    else
        freq -= 0.1;

    if (freq < 87.5)
        freq = 87.5;
    if (freq > 108.0)
        freq = 108.0;

    char new_freq_str[10];
    snprintf(new_freq_str, sizeof(new_freq_str), "%.1f", freq);
    gtk_editable_set_text(GTK_EDITABLE(app->frequency_entry), new_freq_str);

    app->current_frequency = (int)(freq * 100);
    int ret = fm_tune(app->fd, app->current_frequency, FM_BAND_UE); // maybe make configurable in a settings page
    if (ret < 0) {
        append_to_output(app, "Error tuning to new frequency");
    } else {
        update_frequency_display(app, freq);
        append_to_output(app, "Tuned to %.1f MHz", freq);
    }
}

static void on_preset_clicked(GtkButton *button, gpointer user_data) {
    FMRadioApp *app = (FMRadioApp *)g_object_get_data(G_OBJECT(button), "app");
    int preset_number = GPOINTER_TO_INT(user_data);
    // make it configurable i guess? my dads car had the 1 2 3 4 5 buttons and they were configurable
    append_to_output(app, "Preset %d clicked", preset_number);
}

static void on_seek_clicked(GtkButton *button, gpointer user_data) {
    FMRadioApp *app = (FMRadioApp *)g_object_get_data(G_OBJECT(button), "app");
    int direction = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "direction"));
    int ret;
    int freq = app->current_frequency;
    float freq_formatted;

    // 1 for up, 0 for down
    ret = fm_seek(app->fd, &freq, FM_BAND_UE, direction, FM_SEEKTH_LEVEL_DEFAULT);

    if (ret < 0) {
        append_to_output(app, "Error seeking to new frequency");
        return;
    }

    app->current_frequency = freq;
    freq_formatted = freq / 100.0;

    char freq_str[10];
    snprintf(freq_str, sizeof(freq_str), "%.1f", freq_formatted);
    gtk_editable_set_text(GTK_EDITABLE(app->frequency_entry), freq_str);

    update_frequency_display(app, freq_formatted);
    append_to_output(app, "Seeked to %.1f MHz", freq_formatted);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkBuilder *builder;
    GtkWidget *window;
    FMRadioApp *radio_app = g_new0(FMRadioApp, 1);

    radio_app->current_frequency = 8750;
    radio_app->is_muted = FALSE;
    radio_app->timeout_id = 0;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "fmradio.ui", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtk_window_set_application(GTK_WINDOW(window), app);

    radio_app->frequency_display = GTK_WIDGET(gtk_builder_get_object(builder, "frequency_display"));
    radio_app->frequency_entry = GTK_WIDGET(gtk_builder_get_object(builder, "frequency_entry"));
    radio_app->start_button = GTK_WIDGET(gtk_builder_get_object(builder, "start_button"));
    radio_app->stop_button = GTK_WIDGET(gtk_builder_get_object(builder, "stop_button"));
    radio_app->output_text_view = GTK_WIDGET(gtk_builder_get_object(builder, "output_text_view"));
    radio_app->volume_scale = GTK_WIDGET(gtk_builder_get_object(builder, "volume_scale"));
    radio_app->tune_up_button = GTK_WIDGET(gtk_builder_get_object(builder, "tune_up_button"));
    radio_app->tune_down_button = GTK_WIDGET(gtk_builder_get_object(builder, "tune_down_button"));
    radio_app->seek_up_button = GTK_WIDGET(gtk_builder_get_object(builder, "seek_up_button"));
    radio_app->seek_down_button = GTK_WIDGET(gtk_builder_get_object(builder, "seek_down_button"));
    radio_app->mute_button = GTK_WIDGET(gtk_builder_get_object(builder, "mute_button"));

    radio_app->output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(radio_app->output_text_view));

    g_signal_connect(radio_app->frequency_entry, "changed", G_CALLBACK(on_frequency_entry_changed), radio_app);
    g_signal_connect(radio_app->start_button, "clicked", G_CALLBACK(on_start_clicked), radio_app);
    g_signal_connect(radio_app->stop_button, "clicked", G_CALLBACK(on_stop_clicked), radio_app);
    g_signal_connect(radio_app->volume_scale, "value-changed", G_CALLBACK(on_volume_changed), radio_app);
    g_signal_connect(radio_app->tune_up_button, "clicked", G_CALLBACK(on_tune_clicked), radio_app);
    g_signal_connect(radio_app->tune_down_button, "clicked", G_CALLBACK(on_tune_clicked), radio_app);
    g_signal_connect(radio_app->mute_button, "toggled", G_CALLBACK(on_mute_toggled), radio_app);

    g_object_set_data(G_OBJECT(radio_app->seek_up_button), "app", radio_app);
    g_object_set_data(G_OBJECT(radio_app->seek_up_button), "direction", GINT_TO_POINTER(1));
    g_object_set_data(G_OBJECT(radio_app->seek_down_button), "app", radio_app);
    g_object_set_data(G_OBJECT(radio_app->seek_down_button), "direction", GINT_TO_POINTER(0));

    g_signal_connect(radio_app->seek_up_button, "clicked", G_CALLBACK(on_seek_clicked), NULL);
    g_signal_connect(radio_app->seek_down_button, "clicked", G_CALLBACK(on_seek_clicked), NULL);

    for (int i = 0; i < 5; i++) {
        char button_name[20];
        snprintf(button_name, sizeof(button_name), "preset_%d", i+1);
        radio_app->preset_buttons[i] = GTK_WIDGET(gtk_builder_get_object(builder, button_name));

        g_object_set_data(G_OBJECT(radio_app->preset_buttons[i]), "app", radio_app);
        g_signal_connect(radio_app->preset_buttons[i], "clicked", G_CALLBACK(on_preset_clicked), GINT_TO_POINTER(i+1));
    }

    gtk_widget_set_sensitive(radio_app->tune_up_button, FALSE);
    gtk_widget_set_sensitive(radio_app->tune_down_button, FALSE);
    gtk_widget_set_sensitive(radio_app->seek_up_button, FALSE);
    gtk_widget_set_sensitive(radio_app->seek_down_button, FALSE);
    gtk_widget_set_sensitive(radio_app->volume_scale, FALSE);
    gtk_widget_set_sensitive(radio_app->mute_button, FALSE);
    gtk_widget_set_sensitive(radio_app->stop_button, FALSE);

    for (int i = 0; i < 5; i++) {
        gtk_widget_set_sensitive(radio_app->preset_buttons[i], FALSE);
    }

    g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_free), radio_app);

    handle_start_sensitivity(radio_app);

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
