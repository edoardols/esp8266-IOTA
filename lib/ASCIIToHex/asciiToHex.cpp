
#include <Arduino.h>

String asciiToHexdecimal(String ascii) {

    /***** debug ******/
    Serial.println("ASCII: " + ascii);
    /***** end debug ******/

    // buffer to store hext representation
    // twice the size of the text to convert plus space for terminating nul character
    char buffer[sizeof(ascii) * 2 + 1];
    // 'clear' buffer
    buffer[0] = '\0';

    // loop through character array
    for (uint8_t cnt = 0; cnt < ascii.length(); cnt++) {
        // conver each character to its hex representation
        itoa(ascii[cnt], &buffer[cnt * 2], 16);
    }

    /***** debug ******/
    Serial.print("BUFFER: ");
    Serial.println(buffer);
    /***** end debug ******/

    return buffer;
}