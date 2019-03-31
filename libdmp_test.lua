#!/usr/bin/env lua

local dmp=require"libdmp"

local msg=dmp.init()

local devices = dmp.get_devices_name()
for i,v in ipairs(devices) do print("device #"..i,v) end

for t=1,4 do
  local streamer, res = dmp.load("cerror-dreidl.mod")

  local i=50
  while res and i>0 do
    dmp.play(streamer)
    dmp.usleep(1e6/25)
    i=i-1
end

  print (string.format ("stop %s",t<4 and "and restart" or ""))
  dmp.stop(streamer)
end

dmp.exit( streamer)
