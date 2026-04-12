#ifndef QV2APVENCODER_H
#define QV2APVENCODER_H

#include "Qv2Component.h"
#include "Qv2Buffer.h"
#include "Qv2Params.h"
#include "oapv.h"

class Qv2ApvEncoder : public Qv2Component {
public:
    Qv2ApvEncoder();
    ~Qv2ApvEncoder() override;

    // Qv2Component overrides
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

#endif // QV2APVENCODER_H
