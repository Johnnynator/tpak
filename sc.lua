function IsPC() return true end
function IsMac() return false end
function IsLinux() return false end
function IsX360() return false end
function IsPS3() return false end

function SystemGetGlobalObject()
	return
end

function Vec2(x,y) return {x = x, y = y} end
function Vec3()	return end
function Quat()	return end
function u64(a) return a end

function GetU64Time()
	return {
		l = 1,
		h = 1
	}
end

function math.mod(a, b)
	return math.fmod(a, b)
end

function GetCvarValue()
	return
end

function IsAppInPrepareSharedDataMode()
	return false
end

SystemExecFile("scripts/m3dsys.lua")
SystemExecFile("gamedata/physics/main.lua")
SystemExecFile("gamedata/materials/main.lua")
SystemExecFile("scripts/ai/main.lua")
SystemExecFile("gamedata/def/main.lua")
