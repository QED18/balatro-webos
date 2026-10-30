-- Original no-assets LÖVE 11.5 smoke test. It intentionally has no Balatro
-- code, art, audio, archives, save data, or network functionality.
local eventLog = { "launch" }
local saveCount = 0
local toneStarted = false

local function record(message)
  table.insert(eventLog, 1, message)
  while #eventLog > 7 do table.remove(eventLog) end
  print("[smoke] " .. message)
end

local function readSave()
  if love.filesystem.getInfo("smoke-count.txt") then
    saveCount = tonumber(love.filesystem.read("smoke-count.txt")) or 0
  end
  record("save read: " .. saveCount)
end

local function writeSave()
  saveCount = saveCount + 1
  assert(love.filesystem.write("smoke-count.txt", tostring(saveCount)))
  record("save written: " .. saveCount)
end

function love.load()
  love.window.setTitle("Balatro webOS native smoke test")
  love.graphics.setBackgroundColor(0.035, 0.055, 0.10)
  love.filesystem.setIdentity("balatro-webos-smoke")
  readSave()
  local samples, rate = 11025, 11025
  local sound = love.sound.newSoundData(samples, rate, 16, 1)
  for i = 0, samples - 1 do sound:setSample(i, math.sin(i * 2 * math.pi * 440 / rate) * 0.12) end
  local source = love.audio.newSource(sound)
  source:play()
  toneStarted = true
  record("original 440 Hz audio requested")
end

function love.keypressed(key)
  local names = { up = "D-pad Up", down = "D-pad Down", left = "D-pad Left", right = "D-pad Right", ["return"] = "OK", x = "Back" }
  record("input: " .. (names[key] or key))
  if key == "return" then writeSave() end
  if key == "x" or key == "escape" then love.event.quit() end
end

function love.draw()
  local width = love.graphics.getWidth()
  love.graphics.setColor(0.10, 0.75, 0.88)
  love.graphics.rectangle("fill", 64, 52, width - 128, 9)
  love.graphics.setColor(0.94, 0.96, 1.0)
  love.graphics.printf("Native LÖVE 11.5 smoke test", 64, 92, width - 128, "center")
  love.graphics.setColor(0.72, 0.80, 0.92)
  love.graphics.printf("D-pad logs input • OK writes a save • Back exits", 64, 134, width - 128, "center")
  love.graphics.printf("save counter: " .. saveCount .. " • audio: " .. (toneStarted and "requested" or "not started"), 64, 174, width - 128, "center")
  love.graphics.setColor(0.94, 0.96, 1.0)
  for i, message in ipairs(eventLog) do love.graphics.print(message, 84, 230 + (i - 1) * 34) end
end
