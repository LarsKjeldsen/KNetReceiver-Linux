#include <cstdlib>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <string>
#include <mysql.h>
#include "KNetDatabase.h"
#include <time.h>
#include <iostream>
#include <csignal>
#include <mosquitto.h>
#include "main.h"


using namespace std;

#define CONFIG_TIMEOUT 10000 // 10 sek.
#define SETTING_TIMEOUT 500 // 500 ms

// Address of our node
const uint16_t this_node = 0;

KNetDatabase db;
time_t timer;


void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	string topic;
	time_t  t;

	topic = message->topic;

	topic.erase(0, 5);  // Skip first, is always KNet/.

	for (uint i = 0; i < topic.length(); i++)
		if (topic[i] == '/')
			topic[i] = '_';

	t = time(&t);
	struct  tm tm = *localtime(&t);
	char c[20];
	strftime(c, sizeof(c), "%T %F ", &tm);

	printf("Message received : %s - %s - %s\n", c, topic.c_str(), (char*)(message->payload));
	db.InsertReadings(topic.c_str(), (char *)(message->payload));
}


void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	if (!result) {
		/* Subscribe to broker information topics on successful connect. */
		mosquitto_subscribe(mosq, NULL, "KNet/#", 0);
	}
	else {
		fprintf(stderr, "Connect failed\n");
	}
}


void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	int i;

	printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for (i = 1; i<qos_count; i++) {
		printf(", %d", granted_qos[i]);
	}
	printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	if (level != MOSQ_LOG_NOTICE && level != MOSQ_LOG_DEBUG)
		fprintf(stderr, "Callback %d - %s\n", level, str);
}


int main(int argc, char** argv)
{
	const char *host = "192.168.1.21";
	int port = 1883;
	int keepalive = 60;
	bool clean_session = true;
	struct mosquitto *mosq = NULL;

	mosquitto_lib_init();
	mosq = mosquitto_new("KNet-Receiver", clean_session, NULL);
	if (!mosq) {

		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	mosquitto_log_callback_set(mosq, my_log_callback);
	mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_message_callback_set(mosq, my_message_callback);
	mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);

	if (mosquitto_connect(mosq, host, port, keepalive)) {
		fprintf(stderr, "Unable to connect.\n");
		return 1;
	}

	printf("Welcome to KNet Receiver\n");
	
	mosquitto_loop_forever(mosq, 1000, 1);

	return 0;
}
