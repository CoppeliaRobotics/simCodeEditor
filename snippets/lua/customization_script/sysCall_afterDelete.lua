-- using sim-2

function sysCall_afterDelete(inData)
    -- After one or several objects were deleted. Can be reentrant
    for i = 1, #inData.objectHandleList do
        print("Object with handle " .. inData.objectHandleList[i] .. " was deleted")
    end
end

