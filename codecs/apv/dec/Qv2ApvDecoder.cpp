#include "Qv2ApvDecoder.h"

Qv2ApvDecoder::Qv2ApvDecoder() {
    mName = "qv2.apv.decoder";
    setState(UNINITIALIZED);
}

Qv2ApvDecoder::~Qv2ApvDecoder() {
    release();
}

std::string Qv2ApvDecoder::getVersion() const {
    unsigned int ver_num;
    const char* ver_str = oapv_version(&ver_num);
    return std::string("OpenAPV ") + ver_str;
}

Qv2Status Qv2ApvDecoder::configure(const std::vector<Qv2Param*>& params) {
    // TODO: Implement configuration logic
    setState(CONFIGURED);
    return QV2_OK;
}

Qv2Status Qv2ApvDecoder::query(std::vector<Qv2Param*>& params) const {
    // TODO: Implement query logic
    return QV2_OK;
}

Qv2Status Qv2ApvDecoder::queue(std::vector<std::unique_ptr<Qv2Work>> items) {
    // TODO: Implement decoding logic
    return QV2_OK;
}

Qv2Status Qv2ApvDecoder::start() {
    // TODO: Initialize decoder
    setState(RUNNING);
    return QV2_OK;
}

Qv2Status Qv2ApvDecoder::stop() {
    // TODO: Stop decoding process
    setState(STOPPED);
    return QV2_OK;
}

Qv2Status Qv2ApvDecoder::flush() {
    // TODO: Flush remaining frames
    return QV2_OK;
}

void Qv2ApvDecoder::onStateChanged(State state) {

}

void Qv2ApvDecoder::onRelease() {
    // TODO: Cleanup resources
}
