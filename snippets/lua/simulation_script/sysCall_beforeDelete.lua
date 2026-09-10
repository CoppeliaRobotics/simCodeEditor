-- using sim-2

function sysCall_beforeDelete(inData)
    -- Before one or several objects will be deleted. Can be reentrant
    for i = 1, #inData.objectList do
        print("Object with handle " .. inData.objectList[i].handle .. " will be deleted")
    end
end

