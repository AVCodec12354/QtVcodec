#ifndef QV2APVDECODER_H
#define QV2APVDECODER_H

#include "Qv2Component.h"
#include "Qv2Buffer.h"
#include "Qv2Params.h"
#include "oapv.h"

class Qv2ApvDecoder : public Qv2Component {
public:
    Qv2ApvDecoder();
    ~Qv2ApvDecoder() override;

    // Qv2Component overrides
    std::string getVersion() const override;
    Qv2Status configure(const std::vector<Qv2Param*>& params) override;
    Qv2Status query(std::vector<Qv2Param*>& params) const override;
    Qv2Status queue(std::vector<std::unique_ptr<Qv2Work>> items) override;
    Qv2Status start() override;
    Qv2Status stop() override;
    Qv2Status flush() override;

protected:
    void onStateChanged(State state) override;
    void onRelease() override;
};

#endif // QV2APVDECODER_H
