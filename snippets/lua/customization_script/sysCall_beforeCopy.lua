-- using sim-2

function sysCall_beforeCopy(inData)
    -- Before one or several objects will be copied. Can be reentrant
    for i = 1, #inData.objectList do
        print("Object with handle " .. inData.objectList[i].handle .. " will be copied")
    end
end

