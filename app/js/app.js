(function () {
  "use strict";
  var keyOutput = document.getElementById("last-key");
  var result = document.getElementById("test-result");
  var button = document.getElementById("test-button");
  var platform = document.getElementById("platform");
  var presses = 0;
  platform.textContent = /Web0S|WebOS|webOS/i.test(navigator.userAgent) ? "LG webOS detected" : "Desktop preview";
  function keyName(event) {
    var code = event.keyCode || event.which;
    var named = {13:"OK / Enter",37:"Left",38:"Up",39:"Right",40:"Down",461:"Back",27:"Back / Escape",403:"Red",404:"Green",405:"Yellow",406:"Blue"};
    return named[code] || event.key || "Code " + code;
  }
  document.addEventListener("keydown", function (event) {
    keyOutput.textContent = keyName(event);
    if ((event.keyCode === 461 || event.keyCode === 27) && document.activeElement === button) {
      event.preventDefault();
      button.blur();
    }
  });
  button.addEventListener("click", function () {
    presses++;
    result.textContent = "Input confirmed (" + presses + "). This tests the launcher only, not Balatro gameplay.";
  });
  button.focus();
}());
