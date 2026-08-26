#ifndef RAONE_LINK_H
#define RAONE_LINK_H

#include <Arduino.h>

class RaoneLink {
public:
    static RaoneLink& getInstance() {
        static RaoneLink instance;
        return instance;
    }

    void begin();
    bool performBootHandshake();
    bool isConnected();
    
    void sendCommand(const String& cmd);
    void update();
    uint32_t getLiveTxCount() const { return _liveTxCount; }
    uint8_t  getRaoneChannel() const { return _raoneChannel; }
    uint32_t getFailCount()   const { return _failCount; }
    uint16_t getLivePps()     const { return _livePps; }
    bool isBusy() const { return _isBusy; }

private:
    RaoneLink() {}
    bool _connected = false;
    String _rxBuffer;
    uint32_t _liveTxCount = 0;
    uint8_t  _raoneChannel = 0;
    uint32_t _failCount = 0;
    uint16_t _livePps = 0;
    bool _isBusy = false;
};

#endif // RAONE_LINK_H



