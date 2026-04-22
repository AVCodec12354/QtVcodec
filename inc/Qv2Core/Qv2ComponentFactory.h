#ifndef QV2COMPONENTFACTORY_H
#define QV2COMPONENTFACTORY_H

#include "Qv2Component.h"
#include <string>
#include <memory>

class Qv2ComponentFactory {
public:
    enum ComponentType {
        ENCODER_APV,
        DECODER_APV,
    };

    static std::shared_ptr<Qv2Component> createByType(ComponentType type);
    static std::shared_ptr<Qv2Component> createByName(const std::string& name);
};

#endif // QV2COMPONENTFACTORY_H
