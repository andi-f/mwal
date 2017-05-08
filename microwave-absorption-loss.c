// microwave-adsorptions-loss.c - Microwave absorption loss calculate the additional loss by water vapour, oxygen, fog and rain in the atmosphere.

/*
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 * 
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
 
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

//#define DEBUG

#define PROGRAM "Microwave absorptions loss"
#define VERSION  "v 1.2a GTK"
#define COPYRIGHT "2013-2017"
#define COMMENT "Microwave absorption loss calculate the additional loss by water vapour, oxygen, fog and rain in the atmosphere."
#define URI "https://github.com/andi-f/mwal"

typedef struct {
GtkWidget *temperature;					// [°C]
GtkWidget *distance;						// [km]
GtkWidget *rainfall;						// [mm/h]
GtkWidget *rel_humidity;				// [%]
GtkWidget *visibility;					// [km]]
GtkWidget *band;								// [GHz]
GtkWidget *attenuation_vapour;	// [dB]
GtkWidget *attenuation_rain;		// [dB]
GtkWidget *attenuation_oxygen;	// [dB]
GtkWidget *attenuation_fog;			// [dB]
GtkWidget *attenuation;					// [dB]
} mwal_data;

extern double fog_attenuation(double frequency, double T0, double visibility, double distance);
extern double rain_attenuation(double frequency, double rainfall, double distance);
extern double oxygen_attenuation(double frequency, double distance);
extern double vapour_attenuation(double frequency, double temperature, double rel_humidity, double distance);

void calculate(GtkWidget *widget, mwal_data *data);
void about(GtkWidget *widget, gpointer data);
  
int main(int argc, char *argv[]) {
	mwal_data data;
	GtkWidget *window;
	GtkWidget *table;
	GtkWidget *vbox;

	GtkWidget *toolbar;
	GtkToolItem *exit;
	GtkToolItem *info;

	GtkWidget *label_distance;
	GtkWidget *label_temp;
	GtkWidget *label_rainfall;
	GtkWidget *label_humidity;
	GtkWidget *label_visibility;
	GtkWidget *label_band;
	GtkWidget *label_rainfall_attenuation;
	GtkWidget *label_vapour_attenuation;
	GtkWidget *label_oxygen_attenuation;
	GtkWidget *label_fog_attenuation;
	GtkWidget *label_total_attenuation;

	GtkAdjustment *adj_distance, *adj_temperature, *adj_rainfall, *adj_rel_humidity, *adj_visibility;
	GtkWidget *distance, *temperature, *rainfall, *rel_humidity, *visibility; 
  
  GtkWidget *band;
  GtkWidget *band_combo;
	GtkWidget *attenuation_vapour;
	GtkWidget *attenuation_rain;
	GtkWidget *attenuation_oxygen;
	GtkWidget *attenuation_fog;
	GtkWidget *attenuation;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(window), "Microwave absorption loss");
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

	gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);

	exit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exit, -1);
	info = gtk_tool_button_new_from_stock(GTK_STOCK_HELP);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), info, -1);

	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 5);

	g_signal_connect(G_OBJECT(exit), "clicked", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(info), "clicked",	GTK_SIGNAL_FUNC(about), NULL);
	g_signal_connect_swapped(G_OBJECT(window), "destroy",	G_CALLBACK(gtk_main_quit), NULL);

	label_distance = gtk_label_new("Distance/[km]");
	label_temp = gtk_label_new("Temperatur/[°C]");
	label_rainfall = gtk_label_new("Rainfall/[mm/h]");
	label_humidity = gtk_label_new("Relativy humidity/[%]");
	label_visibility = gtk_label_new("Visibility/[km]");
	label_band = gtk_label_new("Band");
	label_vapour_attenuation = gtk_label_new("Vapour attenuation/[dB]");
	label_rainfall_attenuation = gtk_label_new("Rain attenuation/[dB]");
	label_oxygen_attenuation = gtk_label_new("Oxygen attenuation/[dB]");
	label_fog_attenuation = gtk_label_new("Fog attenuation/[dB]");
	label_total_attenuation = gtk_label_new("Total attenuation/[dB]");
	
	adj_distance = (GtkAdjustment *) gtk_adjustment_new(1,0.0,1000.0,1,10,0);
	distance = gtk_spin_button_new(adj_distance,0,0);
	gtk_signal_connect(GTK_OBJECT(distance), "value-changed",GTK_SIGNAL_FUNC(calculate), (gpointer*) &data);

	adj_temperature = (GtkAdjustment *) gtk_adjustment_new(20.0,-20.0,50.0,0.1,1,0);
	temperature = gtk_spin_button_new(adj_temperature,0,1);
	gtk_signal_connect(GTK_OBJECT(temperature), "value-changed",GTK_SIGNAL_FUNC(calculate), (gpointer*) &data);
	
	adj_rainfall = (GtkAdjustment *) gtk_adjustment_new(0.0,0.0,100.0,0.1,1,0);
	rainfall = gtk_spin_button_new(adj_rainfall,0,1);
	gtk_signal_connect(GTK_OBJECT(rainfall), "value-changed",GTK_SIGNAL_FUNC(calculate), (gpointer*) &data);
	
	adj_rel_humidity = (GtkAdjustment *) gtk_adjustment_new(80.0,0.0,100.0,0.1,1,0);
	rel_humidity = gtk_spin_button_new(adj_rel_humidity,0,1);
	gtk_signal_connect(GTK_OBJECT(rel_humidity), "value-changed",GTK_SIGNAL_FUNC(calculate), (gpointer*) &data);

	adj_visibility = (GtkAdjustment *) gtk_adjustment_new(10.00,0.00,10.00,0.01,1.00,0);
	visibility = gtk_spin_button_new(adj_visibility,0,2);
	gtk_signal_connect(GTK_OBJECT(visibility), "value-changed",GTK_SIGNAL_FUNC(calculate), (gpointer*) &data);
	
	band = gtk_fixed_new();
  band_combo = gtk_combo_box_new_text();
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), " 1.3 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), " 2.3 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), " 3.4 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), " 5.7 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), " 10 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), " 24 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), " 47 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), " 76 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), "122 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), "134 GHz");
  gtk_combo_box_append_text(GTK_COMBO_BOX(band_combo), "241 GHz");
	gtk_combo_box_set_active(GTK_COMBO_BOX(band_combo),0);
  gtk_fixed_put(GTK_FIXED(band), band_combo, 0, 0);
	gtk_signal_connect(GTK_OBJECT(band_combo), "changed",GTK_SIGNAL_FUNC(calculate), (gpointer*) &data);
	
	attenuation_vapour = gtk_label_new("0.0");
	attenuation_rain = gtk_label_new("0.0");
	attenuation_oxygen = gtk_label_new("0.0");
	attenuation_fog = gtk_label_new("0.0");
	attenuation = gtk_label_new("0.0");

	table = gtk_table_new(10, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(table), 20);
	
	gtk_box_pack_end (GTK_BOX (vbox), table, TRUE, TRUE, 20);
  
	gtk_table_attach(GTK_TABLE(table), label_distance, 0, 1, 1, 2,
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), distance, 1, 2, 1, 2,
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label_oxygen_attenuation, 2, 3, 1, 2, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), attenuation_oxygen, 3, 4, 1, 2, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	gtk_table_attach(GTK_TABLE(table), label_rainfall, 0, 1, 2, 3, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), rainfall, 1, 2, 2, 3, 
			GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label_rainfall_attenuation, 2, 3, 2, 3, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), attenuation_rain, 3, 4, 2, 3, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
		
	gtk_table_attach(GTK_TABLE(table), label_temp, 0, 1, 3, 4, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), temperature, 1, 2, 3,4, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
		
	gtk_table_attach(GTK_TABLE(table), label_humidity, 0, 1, 4, 5, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), rel_humidity, 1, 2, 4, 5, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label_vapour_attenuation, 2, 3, 4, 5, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), attenuation_vapour, 3, 4, 4, 5, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	gtk_table_attach(GTK_TABLE(table), label_visibility, 0, 1, 5, 6, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), visibility, 1, 2, 5, 6, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label_fog_attenuation, 2, 3, 5, 6, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), attenuation_fog, 3, 4, 5, 6, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	gtk_table_attach(GTK_TABLE(table), label_band, 0, 1, 6, 7, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), band, 1, 2, 6, 7, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
	gtk_table_attach(GTK_TABLE(table), label_total_attenuation, 2, 3, 6, 7, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
		gtk_table_attach(GTK_TABLE(table), attenuation, 3, 4, 6, 7, 
		GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	data.temperature = temperature;
	data.distance = distance;
	data.rel_humidity = rel_humidity;
	data.visibility = visibility;
	data.band = band_combo;
	data.rainfall = rainfall;
	data.attenuation_vapour = attenuation_vapour;
	data.attenuation_rain = attenuation_rain;
	data.attenuation_oxygen = attenuation_oxygen;
	data.attenuation_fog = attenuation_fog;
	data.attenuation = attenuation;

	gtk_widget_show_all(window);

	g_signal_connect(window, "destroy",
		G_CALLBACK(gtk_main_quit), NULL);

	gtk_main();

	return 0;
}

void calculate(GtkWidget *widget, mwal_data *data)	{

	double att_vapour;		//vapour attenuation [dB]
	double att_rain;			//rain attenuation [dB]
	double att_oxygen;		//oxygen attenuation [dB]
	double att_fog;				//oxygen attenuation [dB]
	double att;						//sum attenuation [dB]
	double W;							//weight of water in saturted air[g/m³]
	char buffer[10];
	double frequency =1.296;
	
	gdouble temperature = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->temperature));
	gdouble distance = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->distance));
	gdouble rainfall = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->rainfall));
	gdouble rel_humidity = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->rel_humidity));
	gdouble visibility = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->visibility));
	gchar *band =  gtk_combo_box_get_active_text(GTK_COMBO_BOX(data->band));

	W = 4.85 + 0.33 * temperature + 0.01 * pow(temperature,2) + 0.00023 * pow(temperature,3);
	
	W = W * rel_humidity/100.0;

	if (strcmp(band," 1.3 GHz") == 0) {
		frequency = 1.296;
	}
	else 	
		if (strcmp(band," 2.3 GHz") == 0) {
		frequency = 2.320;
	}
	else 	
		if (strcmp(band," 3.4 GHz") == 0) {
		frequency = 3.400;
	}
	else 	
		if (strcmp(band," 5.6 GHz") == 0) {
		frequency =  5.760;
	}
	else 	
	if (strcmp(band," 10 GHz") == 0) {
		frequency = 10.368;
	}
	else 	
	if (strcmp(band," 24 GHz") == 0) {
		frequency = 24.048;
	}
	else 
	if (strcmp(band," 47 GHz") == 0) {
		frequency = 47.088;
	}
	else 
	if (strcmp(band," 76 GHz") == 0) {
		frequency = 76.032;
	}
	else 
		if (strcmp(band,"122 GHz") == 0) {
		frequency = 122.250;
	}
	else 
		if (strcmp(band,"134 GHz") == 0) {
		frequency = 134.928;		
	}
	else 
	if (strcmp(band,"241 GHz")== 0) {
		frequency = 241.920;						
	}

	att_oxygen = oxygen_attenuation(frequency,distance);
	att_fog = fog_attenuation(frequency, temperature, visibility, distance);
	att_rain = rain_attenuation(frequency, rainfall, distance);
  att_vapour = vapour_attenuation(frequency, temperature, rel_humidity, distance);

#ifdef DEBUG
	g_print("Frequency %3.3f GHz \n\r",frequency);
	g_print("Distance %f km\n\r",distance);	
	g_print("W %2.1f g/m³ \n\r",W);
	g_print("Rainfall %2.1f g/m³ \n\r",rainfall);
	g_print("Att vapour %3.2f dB \n\r",att_vapour);
	g_print("Att rain %3.2f dB \n\r",att_rain);
	g_print("Att oxygen %3.2f dB \n\r",att_oxygen);
	g_print("Att fog %3.2f dB \n\r",att_fog);
#endif

	att = att_vapour + att_rain + att_oxygen + att_fog;

	sprintf(buffer,"%3.1f",att_vapour);
	gtk_label_set_text(GTK_LABEL(data->attenuation_vapour),buffer); 

	sprintf(buffer,"%3.1f",att_rain);
	gtk_label_set_text(GTK_LABEL(data->attenuation_rain),buffer); 

	sprintf(buffer,"%3.1f",att_oxygen);
	gtk_label_set_text(GTK_LABEL(data->attenuation_oxygen),buffer); 

	sprintf(buffer,"%3.1f",att_fog);
	gtk_label_set_text(GTK_LABEL(data->attenuation_fog),buffer); 
	
	sprintf(buffer,"%3.1f",att);
	gtk_label_set_text(GTK_LABEL(data->attenuation),buffer); 
}

// Callback for about
void about(GtkWidget *widget, gpointer data) {
  GtkWidget *dialog = gtk_about_dialog_new();
  gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), PROGRAM);
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION); 
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), COPYRIGHT);
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), COMMENT);
  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), URI);
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
 }  
