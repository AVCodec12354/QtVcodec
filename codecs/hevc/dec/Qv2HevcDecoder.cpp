#include "Qv2HevcDecoder.h"

Qv2HevcDecoder::Qv2HevcDecoder() {
    mName = "qv2.hevc.decoder";
    setState(UNINITIALIZED);
}

Qv2HevcDecoder::~Qv2HevcDecoder() {
    release();
}

std::string Qv2HevcDecoder::getVersion() const {
    // TODO: Return actual HEVC decoder library version
    return "HEVC Decoder v1.0 (Stub)";
}

Qv2Status Qv2HevcDecoder::configure(const std::vector<Qv2Param*>& params) {
    // TODO: Implement HEVC decoding configuration logic
    setState(CONFIGURED);
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::query(std::vector<Qv2Param*>& params) const {
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::queue(std::vector<std::unique_ptr<Qv2Work>> items) {
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::start() {
    setState(RUNNING);
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::stop() {
    setState(STOPPED);
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::flush() {
    return QV2_OK;
}

void Qv2HevcDecoder::onStateChanged(State state) {

}

void Qv2HevcDecoder::onRelease() {
}
