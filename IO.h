#ifndef IO_H
#define IO_H

#define TX_PIN D3
#define RX_PIN D4

class IO {
    public:
    
    IO()
    {
        pinMode(TX_PIN, OUTPUT);
        pinMode(RX_PIN, INPUT);
    }

    inline void write(byte v) { digitalWrite(TX_PIN, v); }
    inline bool read() { return digitalRead(RX_PIN);}
};

#endif
