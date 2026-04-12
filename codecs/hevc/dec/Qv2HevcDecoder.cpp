#include "Qv2HevcDecoder.h"
#include <QDebug>

Qv2HevcDecoder::Qv2HevcDecoder() {
    mName = "qv2.hevc.decoder";
    setState(UNINITIALIZED);
}

Qv2HevcDecoder::~Qv2HevcDecoder() {
    release();
}

Qv2Status Qv2HevcDecoder::configure(const std::vector<Qv2Param*>& params) {
    // TODO: Implement HEVC decoding configuration logic
    setState(CONFIGURED);
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::query(std::vector<Qv2Param*>& params) const {
    // TODO: Implement query logic
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::queue(std::vector<std::unique_ptr<Qv2Work>> items) {
    // TODO: Implement HEVC decoding logic
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::start() {
    // TODO: Initialize HEVC decoder
    setState(RUNNING);
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::stop() {
    // TODO: Stop HEVC decoding process
    setState(STOPPED);
    return QV2_OK;
}

Qv2Status Qv2HevcDecoder::flush() {
    // TODO: Flush remaining frames
    return QV2_OK;
}

void Qv2HevcDecoder::onStateChanged(State state) {
    qDebug() << "Qv2HevcDecoder state changed to:" << state;
}

void Qv2HevcDecoder::onRelease() {
    // TODO: Cleanup HEVC decoder resources
}
