#include "Qv2ComponentFactory.h"
#include "apv/enc/Qv2ApvEncoder.h"
#include "apv/dec/Qv2ApvDecoder.h"

std::shared_ptr<Qv2Component> Qv2ComponentFactory::createByType(ComponentType type) {
    switch (type) {
        case ENCODER_APV:
            return std::make_shared<Qv2ApvEncoder>();
        case DECODER_APV:
            return std::make_shared<Qv2ApvDecoder>();
        default:
            return nullptr;
    }
}

std::shared_ptr<Qv2Component> Qv2ComponentFactory::createByName(const std::string& name) {
    if (name == "qv2.apv.encoder") {
        return createByType(ENCODER_APV);
    } else if (name == "qv2.apv.decoder") {
        return createByType(DECODER_APV);
    }
    return nullptr;
}
