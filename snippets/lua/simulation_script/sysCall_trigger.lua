-- using sim-2

function sysCall_trigger(inData)
    -- Called when the attached vision sensor, proximity sensor or force sensor is triggered
    --
    -- inData when attached object is a vision sensor:
    -- inData.visionSensor: the vision sensor
    -- inData.packedPackets: an array of data packets, packed (use sim.unpackFloatTable)
    --
    -- inData when attached object is a proximity sensor:
    -- inData.proximitySensor: the proximity sensor
    -- inData.object: the detected object
    -- inData.point: detected point, relative to sensor frame
    -- inData.normal: normal vector at detected point, relative to sensor frame
    --
    -- inData when attached object is a force sensor:
    -- inData.forceSensor: the force sensor
    -- inData.forceVector: the measured force vector
    -- inData.torqueVector: the measured torque vector
    -- inData.filteredForceVector: the filtered force vector
    -- inData.filteredTorqueVector: the filtered torque vector

    outData = {}
    outData.trigger = true
    return outData
end
