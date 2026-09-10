-- using sim-2

function sysCall_vision(inData)
    -- Can be used for image processing.
    -- inData.sensor: the vision sensor
    -- inData.resolution: resolution of the vision sensor
    -- inData.clippingPlanes: near and far clipping plane of the vision sensor
    -- inData.viewAngle: the view angle if the vision sensor is in perspective operation
    -- inData.orthoSize: the size of the view if the vision sensor is in orthogonal operation
    -- inData.perspective: whether the vision sensor is in perspective operation

    -- e.g.:
    simVision.sensorImgToWorkImg(inData.sensor.handle)
    simVision.intensityScaleOnWorkImg(inData.sensor.handle, 1.0, 0.0, false)
    simVision.workImgToSensorImg(inData.sensor.handle)

    -- or, e.g.:
    local imgHandle = simIM.readFromVisionSensor(inData.sensor.handle)
    local center = {inData.resolution[1] / 2, inData.resolution[2] / 2}
    local radius = (inData.resolution[1] + inData.resolution[2]) / 8
    simIM.circle(imgHandle, center, radius, {255, 0, 255}, 4)
    simIM.writeToVisionSensor(imgHandle, inData.sensor.handle)
    simIM.destroy(imgHandle)

    outData = {}
    outData.trigger = false -- whether the sensor should trigger
    outData.packedPackets = {} -- filters may append packets (in packed form, use sim.app:packArray with float type) to this table
    return outData
end
