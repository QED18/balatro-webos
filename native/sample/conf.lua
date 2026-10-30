-- Request the TV's desktop mode instead of LÖVE's default small window.
-- This keeps the owner setup view edge-to-edge on both 1080p and 4K panels.
function love.conf(t)
  t.window.fullscreen = true
  t.window.fullscreentype = "desktop"
  t.window.vsync = 1
end
