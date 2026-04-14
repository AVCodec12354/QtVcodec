#include "EncoderViewModel.h"
#include "QTLogger.h"

#define LOG_TAG "EncoderViewModel"

EncoderViewModel::EncoderViewModel() {
    // TODO:
}

void EncoderViewModel::start() {
    // TODO
}

void EncoderViewModel::stop() {
    // TODO
}

void EncoderViewModel::setWidth(int value) {
    // TODO
}

void EncoderViewModel::setHeight(int value) {
    // TODO
}

void EncoderViewModel::setFPS(int value) {
    // TODO
}

void EncoderViewModel::setBitDepth(int value) {
    // TODO
}

void EncoderViewModel::setColorSpace(string value) {
    // TODO
}

void EncoderViewModel::enableBitrateABR(bool value) {
    // TODO
}

void EncoderViewModel::setQuantizationParameters(int value) {
    // TODO
}

void EncoderViewModel::setProfile(string value) {
    // TODO
}

void EncoderViewModel::setLevel(int value) {
    // TODO
}

void EncoderViewModel::setFamily(string value) {
    // TODO
}

void EncoderViewModel::setBand(int value) {
    // TODO
}

void EncoderViewModel::setMaxCU(int value) {
    // TODO
}

void EncoderViewModel::setSpeedCU(int value) {
    // TODO
}

void EncoderViewModel::setWidthOfTile(int value) {
    // TODO
}

void EncoderViewModel::setHeightOfTile(int value) {
    // TODO
}

void EncoderViewModel::setPrimaries(string value) {
    // TODO
}

void EncoderViewModel::setTransfer(string value) {
    // TODO
}

void EncoderViewModel::setMatrix(string value) {
    // TODO
}

void EncoderViewModel::setRange(string value) {
    // TODO
}

void EncoderViewModel::setMasteringDisplay(int value) {
    // TODO
}

void EncoderViewModel::setContentLightLevel(int value) {
    // TODO
}

void EncoderViewModel::testEncoder() {
    QTInfo(LOG_TAG, "Saving configuration and creating encoder...");

    mEncoder = Qv2ComponentFactory::createByType(Qv2ComponentFactory::ENCODER_APV);

    if (!mEncoder) {
        QTError(LOG_TAG, "Failed to create APV Encoder!");
        return;
    }
    QTDebug(LOG_TAG, "APV Encoder version: "+mEncoder->getVersion());

}