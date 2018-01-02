/**
 *
 * @file ESP8266HTTPUpdate.h
 * @date 21.06.2015
 * @author Markus Sattler
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the ESP8266 Http Updater.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef ESP8266HTTPUPDATEIASMOD_H_
	#define ESP8266HTTPUPDATEIASMOD_H_

	#include <Arduino.h>
	#include <ESP8266WiFi.h>
	#include <WiFiClient.h>
	#include <WiFiUdp.h>
	#include <ESP8266HTTPClient.h>

	#define DEBUG_LVL 2	// 1, 2 or 3
	#if DEBUG_LVL >= 1
		#define         DEBUG_PRINT(x)    { Serial.print(x);  }
		#define         DEBUG_PRINTF(...) { Serial.printf(__VA_ARGS__);  }
		#define         DEBUG_PRINTF_P(...) { Serial.printf_P(__VA_ARGS__);  }
		#define         DEBUG_PRINTLN(x)  { Serial.println(x); }
	#endif

	class ESP8266HTTPUpdate{
		
		public:

			void rebootOnUpdate(bool reboot){
				_rebootOnUpdate = reboot;
			}

			void handleUpdate(HTTPClient& http, int len, bool spiffs);


		protected:

			bool runUpdate(Stream& in, uint32_t size, String md5, int command = U_FLASH);
			bool _rebootOnUpdate = true;
		
	};

#endif
