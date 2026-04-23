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

    std::string getVersion() const override;

    Qv2Status configure(const std::vector<Qv2Param *> &params) override;

    Qv2Status query(std::vector<Qv2Param *> &params) const override;

    Qv2Status queue(std::vector <std::unique_ptr<Qv2Work>> items) override;

    Qv2Status start() override;

    Qv2Status stop() override;

    Qv2Status flush() override;

protected:
    void onStateChanged(State state) override;

    void onRelease() override;

private:
    void showEncoderParams(oapve_cdesc_t *cdsc) const;

    oapve_t mEncoderId = nullptr;
    oapvm_t mMetaDataId = nullptr;
    uint8_t *mBitstreamBuf = nullptr;

    std::unique_ptr <oapve_cdesc_t> mCodecDesc;
    oapv_frms_t mInputFrames;
    oapv_frms_t mReconFrames;
    oapv_bitb_t mBitb;
    oapve_stat_t mStat;

    bool mIsRec = false;
};

#endif // QV2APVENCODER_H
