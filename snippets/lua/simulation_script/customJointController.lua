function sysCall_init()
    sim = require('sim')
    joint = sim.getObject('..')

    -- Desired control mode ('position', 'velocity' or 'force'):
    jointMode = 'position'

    -- in position mode: sets the maximum force/torque
    -- in velocity mode: sets the maximum force/torque
    -- in force/torque mode: sets the desired force/torque
    sim.setJointTargetForce(joint, 20.0)

    if jointMode ~= 'force' then
        -- in position mode: sets the maximum velocity
        -- in velocity mode: sets the desired velocity
        -- in force/torque mode: has no effect
        sim.setJointTargetVelocity(joint, 90.0 * math.pi / 180.0)

        -- in position mode: sets the desired position
        -- in velocity mode: has no effect
        -- in force/torque mode: has no effect
        sim.setJointTargetPosition(joint, 90.0 * math.pi / 180.0)
    end
    
    -- The PID values for the position controller's first stage:
    K_vel_p = 50.0
    K_vel_d = 0.25
    K_vel_i = 0.0

    -- The PID values for the velocity controller (and the position controller's second stage):
    K_accel_p = 300.0
    K_accel_d = 0.05
    K_accel_i = 0.0
    
    -- An overall control value scaling factor, linked to the attached mass (normally that value is obtained via inverse dynamics, and is not constant):
    K_mass = 1.0

    jointData = {}
    jointData.prevPosError  = 0.0
    jointData.cumulPosError = 0.0
    jointData.prevVelError = 0.0
    jointData.cumulVelError = 0.0

end

function sysCall_joint(inData)
    local jointVel_desired = 0.0

    if jointMode == 'position' then
        -- The position controller has 2 stages: the first stage regulates the
        -- corresponding required velocity (then in stage 2 regulates the
        -- corresponding required acceleration):
        -------------------------------------------------------------------------------
        local jointPosError_deriv = (inData.error - jointData.prevPosError) / inData.dt
        jointData.cumulPosError = jointData.cumulPosError + inData.error * inData.dt
        jointVel_desired = K_vel_p * inData.error + K_vel_d * jointPosError_deriv + K_vel_i * jointData.cumulPosError

        -- Clamp velocity:
        jointVel_desired = math.min(jointVel_desired, math.abs(inData.targetVel))
        jointVel_desired = math.max(jointVel_desired, -math.abs(inData.targetVel))
        -------------------------------------------------------------------------------
    end

    local jointVelError = 0.0
    local forceToApply = 0.0

    if jointMode == 'position' or jointMode == 'velocity' then
        -- The velocity controller (and stage 2 position controller) regulates the
        -- corresponding required acceleration:
        -----------------------------------------------------------------------------
        if jointMode == 'velocity' then
            jointVel_desired = inData.targetVel
        end
        jointVelError = jointVel_desired - inData.vel
        local jointVelError_deriv = (jointVelError - jointData.prevVelError) / inData.dt
        jointData.cumulVelError = jointData.cumulVelError + jointVelError * inData.dt

        local jointAccel_desired = K_accel_p * jointVelError + K_accel_d * jointVelError_deriv + K_accel_i * jointData.cumulVelError

        forceToApply = jointAccel_desired * K_mass

        -- Clamp force/torque:
        forceToApply = math.min(forceToApply, math.abs(inData.force))
        forceToApply = math.max(forceToApply, -math.abs(inData.force))
        -----------------------------------------------------------------------------
    end

    if jointMode == 'force' then
        forceToApply = inData.force
    end
    
    jointData.prevPosError = inData.error
    jointData.prevVelError = jointVelError

    return {force = forceToApply}
end
