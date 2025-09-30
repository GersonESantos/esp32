#ifndef _TEMPERATURAUMIDADELUMINOSIDADE_H_
#define _TEMPERATURAUMIDADELUMINOSIDADE_H_

#include <SinricProDevice.h>
#include <Capabilities/TemperatureSensor.h>
#include <Capabilities/PowerStateController.h>
#include <Capabilities/RangeController.h>

class TemperaturaUmidadeLuminosidade 
: public SinricProDevice
, public TemperatureSensor<TemperaturaUmidadeLuminosidade>
, public PowerStateController<TemperaturaUmidadeLuminosidade>
, public RangeController<TemperaturaUmidadeLuminosidade> {
  friend class TemperatureSensor<TemperaturaUmidadeLuminosidade>;
  friend class PowerStateController<TemperaturaUmidadeLuminosidade>;
  friend class RangeController<TemperaturaUmidadeLuminosidade>;
public:
  TemperaturaUmidadeLuminosidade(const String &deviceId) : SinricProDevice(deviceId, "TemperaturaUmidadeLuminosidade") {};
};

#endif
