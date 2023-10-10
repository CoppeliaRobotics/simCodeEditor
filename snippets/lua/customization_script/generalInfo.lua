-- A customization script is intended to be used to customize a scene in
-- various ways, mainly when simulation is not running. When simulation is running,
-- do not use customization scripts, but rather child scripts if possible
--
-- if you wish to execute code contained in an external file,
-- use the require-directive, e.g.:
--
-- require 'myExternalFile'
--
-- Above will look for <CoppeliaSim executable path>/myExternalFile.lua or
-- <CoppeliaSim executable path>/lua/myExternalFile.lua
-- (the file can be opened in this editor with the popup menu over
-- the file name)
--
--
-- Make sure you read the section on "Accessing scene objects programmatically"
-- For instance, if you wish to retrieve the handle of a scene object, use following instruction:
--
-- handle = sim.getObject('./sceneObjectName') -- will search in the current scene object hierarchy
--
-- Refer to the user manual and the code snippets, for additional details and infos about
-- available callback functions
