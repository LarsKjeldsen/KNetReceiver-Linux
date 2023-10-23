#include <cstdlib>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <string>
#include <mysql/mysql.h>
#include "KNetDatabase.h"
#include <time.h>
#include <iostream>
#include <csignal>
#include <mosquitto.h>
#include "main.h"


using namespace std;

#define CONFIG_TIMEOUT 10000 // 10 sek.
#define SETTING_TIMEOUT 500 // 500 ms

KNetDatabase db;
time_t timer;

char * get_time()
{
	char *s;
	time_t t;
	t = time(NULL);
	struct  tm* tm = localtime(&t);
	s = asctime(tm);
	s[strlen(s)-1]='\0';
	return s;
}

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	string topic;

	topic = message->topic;

	topic.erase(0, 5);  // Skip first, is always KNet/.

	for (uint i = 0; i < topic.length(); i++)
		if (topic[i] == '/')
			topic[i] = '_';

	printf("%s : Message received : %s : %s\n", get_time(), topic.c_str(), (char*)(message->payload));
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
		fprintf(stderr, "%s : Callback %d - %s\n", get_time(), level, str);
}


int main(int argc, char** argv)
{
	const char *host = "192.168.1.22";
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
		fprintf(stderr, "%s : Unable to connect.\n", get_time());
		return 1;
	}

	printf("Welcome to KNet Receiver\n");
	
	mosquitto_loop_forever(mosq, 10000, 1);

	return 0;
}
