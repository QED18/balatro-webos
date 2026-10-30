-- Original, game-file-free first-run display for Balatro for webOS.
-- It deliberately uses only LÖVE drawing primitives: no game art, code,
-- archives, or network code are bundled in this setup archive.
local BASE_W, BASE_H = 1280, 720
local portalUrl = os.getenv("BALATRO_PORTAL_URL") or ""
local portalPin = os.getenv("BALATRO_PORTAL_PIN") or "----"
local transferLabel, transferProgress = "Waiting for your computer", 0
local elapsed, saveCount, toneStarted = 0, 0, false
local fonts = {}

local palette = {
  ink = {0.018, 0.038, 0.045}, felt = {0.025, 0.135, 0.090},
  feltLight = {0.045, 0.235, 0.148}, cyan = {0.24, 0.92, 0.91},
  blue = {0.20, 0.57, 0.94}, crimson = {0.88, 0.17, 0.27},
  gold = {0.96, 0.77, 0.28}, paper = {0.94, 0.95, 0.82},
  shade = {0.005, 0.012, 0.016}
}

local function colour(name, alpha)
  local c = palette[name]
  love.graphics.setColor(c[1], c[2], c[3], alpha or 1)
end

local function font(size)
  return fonts[size]
end

local function readStatus()
  if not love.filesystem.getInfo("portal-status.txt") then return end
  local status = (love.filesystem.read("portal-status.txt")) or ""
  local label, progress = status:match("^(.-)|(%d+)$")
  if label then
    transferLabel = label
    transferProgress = tonumber(progress) or 0
  end
end

local function roundRect(mode, x, y, w, h, radius)
  love.graphics.rectangle(mode, x, y, w, h, radius, radius)
end

local function card(x, y, w, h, suit, caption, accent, tilt)
  love.graphics.push()
  love.graphics.translate(x + w / 2, y + h / 2)
  love.graphics.rotate(tilt)
  love.graphics.translate(-w / 2, -h / 2)
  colour("shade", 0.75)
  roundRect("fill", 8, 11, w, h, 16)
  colour("paper")
  roundRect("fill", 0, 0, w, h, 16)
  colour("ink")
  love.graphics.setLineWidth(5)
  roundRect("line", 8, 8, w - 16, h - 16, 10)
  colour(accent)
  love.graphics.setFont(font(22))
  love.graphics.print(suit, 19, 18)
  love.graphics.print(suit, w - 36, h - 43)
  colour("ink")
  love.graphics.setFont(font(19))
  love.graphics.printf(caption, 18, h * 0.46, w - 36, "center")
  colour(accent, 0.9)
  love.graphics.circle("fill", w / 2, h * 0.67, 18)
  colour("paper")
  love.graphics.circle("line", w / 2, h * 0.67, 28)
  love.graphics.pop()
end

local function title(text, y, size, tint)
  love.graphics.setFont(font(size))
  colour(tint or "paper")
  love.graphics.printf(text, 0, y, BASE_W, "center")
end

function love.load()
  love.window.setTitle("Balatro for webOS")
  love.filesystem.setIdentity("balatro-webos-smoke")
  for _, size in ipairs({15, 16, 17, 18, 19, 20, 22, 24, 31, 35}) do
    fonts[size] = love.graphics.newFont(size)
  end
  if love.filesystem.getInfo("smoke-count.txt") then
    saveCount = tonumber((love.filesystem.read("smoke-count.txt"))) or 0
  end
  -- Keep the tiny original audio-path smoke check without storing media.
  local sound = love.sound.newSoundData(64, 11025, 16, 1)
  local source = love.audio.newSource(sound)
  source:play()
  toneStarted = true
end

function love.update(dt)
  elapsed = elapsed + dt
  readStatus()
end

function love.keypressed(key)
  if key == "escape" or key == "x" then love.event.quit() end
  if key == "return" then
    saveCount = saveCount + 1
    love.filesystem.write("smoke-count.txt", tostring(saveCount))
  end
end

function love.gamepadpressed(_, button)
  if button == "b" then love.event.quit() end
  if button == "a" then
    saveCount = saveCount + 1
    love.filesystem.write("smoke-count.txt", tostring(saveCount))
  end
end

function love.draw()
  local screenW, screenH = love.graphics.getDimensions()
  love.graphics.clear(palette.shade)
  love.graphics.push()
  -- webOS can report a non-16:9 drawable area. Fill the actual panel rather
  -- than leaving a boxed setup screen around the 1280×720 composition.
  love.graphics.scale(screenW / BASE_W, screenH / BASE_H)

  -- Printed-felt table with slow, original neon flourishes.
  colour("felt")
  love.graphics.rectangle("fill", 0, 0, BASE_W, BASE_H)
  for i = 0, 22 do
    local wave = math.sin(elapsed * 0.35 + i * 1.71)
    colour(i % 2 == 0 and "feltLight" or "blue", 0.035)
    love.graphics.setLineWidth(35)
    love.graphics.arc("line", "open", 640 + wave * 180, 380 + math.cos(elapsed * 0.27 + i) * 100, 300 + i * 25, i * 0.42, i * 0.42 + 1.1)
  end
  colour("ink", 0.86)
  love.graphics.rectangle("fill", 0, 0, BASE_W, 92)
  colour("gold")
  love.graphics.rectangle("fill", 0, 86, BASE_W, 6)

  love.graphics.setFont(font(24))
  colour("cyan")
  love.graphics.print("BALATRO", 48, 28)
  colour("paper")
  love.graphics.print("FOR webOS", 194, 32)
  colour("gold")
  love.graphics.printf("OWNER SETUP  •  PRIVATE LAN", 0, 33, BASE_W - 48, "right")

  local bob = math.sin(elapsed * 1.1) * 6
  card(119, 152 + bob, 134, 190, "♠", "YOUR", "cyan", -0.10)
  card(1005, 158 - bob, 134, 190, "♦", "DECK", "crimson", 0.10)

  colour("shade", 0.68)
  roundRect("fill", 296, 125, 688, 484, 28)
  colour("gold")
  love.graphics.setLineWidth(5)
  roundRect("line", 296, 125, 688, 484, 28)
  colour("paper", 0.14)
  love.graphics.rectangle("fill", 314, 143, 652, 5)

  title("BRING YOUR OWN DECK", 161, 35, "paper")
  title("A private setup table for your legal copy", 211, 18, "cyan")
  love.graphics.setFont(font(20))
  colour("paper")
  love.graphics.printf("ON YOUR COMPUTER", 0, 266, BASE_W, "center")
  love.graphics.setFont(font(17))
  colour("paper", 0.84)
  love.graphics.printf("Open the address below, enter this table PIN, then choose", 0, 299, BASE_W, "center")
  colour("gold")
  love.graphics.printf("a supported Balatro game file from your installed copy", 0, 326, BASE_W, "center")

  colour("ink")
  roundRect("fill", 355, 365, 570, 73, 13)
  colour("cyan")
  love.graphics.setLineWidth(3)
  roundRect("line", 355, 365, 570, 73, 13)
  love.graphics.setFont(font(24))
  colour("paper")
  love.graphics.printf(portalUrl ~= "" and portalUrl or "Starting private portal…", 370, 388, 540, "center")

  colour("crimson")
  roundRect("fill", 511, 461, 258, 72, 13)
  colour("paper")
  love.graphics.setFont(font(15))
  love.graphics.printf("TABLE PIN", 511, 473, 258, "center")
  love.graphics.setFont(font(31))
  love.graphics.printf(portalPin, 511, 491, 258, "center")

  colour("paper", 0.20)
  roundRect("fill", 355, 554, 570, 18, 9)
  colour(transferProgress >= 100 and "gold" or "cyan")
  roundRect("fill", 355, 554, 570 * math.max(0, math.min(100, transferProgress)) / 100, 18, 9)
  love.graphics.setFont(font(18))
  colour("paper")
  love.graphics.printf(transferLabel .. "  " .. transferProgress .. "%", 0, 580, BASE_W, "center")

  love.graphics.setFont(font(16))
  colour("paper", 0.78)
  love.graphics.printf("No account • No cloud • Your archive remains on this TV", 0, 642, BASE_W, "center")
  colour("paper", 0.45)
  love.graphics.printf("After a complete transfer: quit and reopen this app.", 0, 670, BASE_W, "center")

  -- Gentle scanline texture, rendered once over the canvas each frame.
  colour("ink", 0.11)
  for y = 0, BASE_H, 4 do love.graphics.rectangle("fill", 0, y, BASE_W, 1) end
  love.graphics.pop()
end
