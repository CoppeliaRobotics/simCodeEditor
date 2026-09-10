-- using sim-2

function sysCall_afterCreate(inData)
    -- After one or several objects were created. Can be reentrant
    for i = 1, #inData.objectList do
        print("Object with handle " .. inData.objectList[i].handle .. " was created")
    end
end

