\ Copyright 2022 Bradley D. Nelson
\
\ Licensed under the Apache License, Version 2.0 (the "License");
\ you may not use this file except in compliance with the License.
\ You may obtain a copy of the License at
\
\     http://www.apache.org/licenses/LICENSE-2.0
\
\ Unless required by applicable law or agreed to in writing, software
\ distributed under the License is distributed on an "AS IS" BASIS,
\ WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
\ See the License for the specific language governing permissions and
\ limitations under the License.

vocabulary web   web definitions

: jseval! ( a n index -- ) 0 call ;

r|
(function(sp) {
  var n = i32[sp>>2]; sp -= 4;
  var a = i32[sp>>2]; sp -= 4;
  var text = GetString(a, n);
  eval(text);
  return sp;
})
| 1 jseval!
: jseval ( a n -- ) 1 call ;

r~
context.inbuffer = [];
context.outbuffer = '';
if (!globalObj.write) {
  function AddMeta(name, content) {
    var meta = document.createElement('meta');
    meta.name = name;
    meta.content = content;
    document.head.appendChild(meta);
  }

  AddMeta('apple-mobile-web-app-capable', 'yes');
  AddMeta('apple-mobile-web-app-status-bar-style', 'black-translucent');
  AddMeta('viewport', 'width=device-width, initial-scale=1.0, ' +
                      'maximum-scale=1.0, user-scalable=no, minimal-ui');

  context.screen = document.getElementById('ueforth');
  if (context.screen === null) {
    context.screen = document.createElement('div');
    context.screen.style.width = '100%';
    document.body.appendChild(context.screen);
  }
  context.filler = document.createElement('div');
  document.body.insertBefore(context.filler, document.body.firstChild);
  context.canvas = document.createElement('canvas');
  context.canvas.width = 1000;
  context.canvas.height = 1000;
  context.canvas.style.width = '1px';
  context.canvas.style.height = '1px';
  context.canvas.style.top = 0;
  context.canvas.style.left = 0;
  context.canvas.style.position = 'fixed';
  context.canvas.style.backgroundColor = '#000';
  context.screen.appendChild(context.canvas);
  context.ctx = context.canvas.getContext('2d');
  context.terminal = document.createElement('pre');
  context.terminal.style.width = '100%';
  context.terminal.style.whiteSpace = 'pre-wrap';
  context.screen.appendChild(context.terminal);

  const TAB = ['&#11134;', '&#11134;', 9, 9, 45];
  const PIPE = ['\\', String.fromCharCode(124), 92, 124, 45];
  const ENTER = ['&#9166;', '&#9166;', 13, 13, 70];
  const CTRL = ['ctrl', 'ctrl', 0, 0, 60];
  const SHIFT = ['&#x21E7;', '&#x21E7', 0, 0, 75];
  const BACKSPACE = ['&#x232B;', '&#x232B;', 8, 8, 60];
  const BACKTICK = [String.fromCharCode(96), String.fromCharCode(126), 96, 126];
  const G15 = ['Gap', 15];
  const G20 = ['Gap', 20];
  const G35 = ['Gap', 35];
  const KEYMAP = [
    '1!', '2@', '3#', '4$', '5%', '6^', '7&', '8*', '9(', '0)', '-_', '=+', BACKSPACE, 'Newline',
    G15, 'qQ', 'wW', 'eE', 'rR', 'tT', 'yY', 'uU', 'iI', 'oO', 'pP', '[{', ']}', PIPE, 'Newline',
    G20, 'aA', 'sS', 'dD', 'fF', 'gG', 'hH', 'jJ', 'kK', 'lL', ';:', '\'"', ENTER, 'Newline',
    G35, 'zZ', 'xX', 'cC', 'vV', 'bB', 'nN', 'mM', ',<', '.>', '/?', BACKTICK, 'Newline',
    SHIFT, [' ', ' ', 32, 32, 9 * 30], SHIFT,
  ];
  const KEY_COLOR = 'linear-gradient(to bottom right, #ccc, #999)';
  const KEY_GOLD = 'linear-gradient(to bottom right, #fc4, #c91)';
  context.keyboard = document.createElement('div');
  var keys = [];
  var shift = 0;
  function UpdateKeys() {
    for (var i = 0; i < keys.length; ++i) {
      keys[i].update();
    }
  }
  function AddKey(item) {
    if (item === 'Newline') {
      var k = document.createElement('br');
      context.keyboard.appendChild(k);
      return;
    }
    var k = document.createElement('button');
    k.style.verticalAlign = 'middle';
    k.style.border = 'none';
    k.style.margin = '0';
    k.style.padding = '0';
    k.style.backgroundImage = 'linear-gradient(to bottom right, #ccc, #999)';
    k.style.width = (100 / 15) + '%';
    k.style.height = '30px';
    if (item[0] === 'Gap') {
      k.style.opacity = 0;
      k.style.width = (100 / 15 * item[1] / 30) + '%';
      context.keyboard.appendChild(k);
      return;
    }
    if (item.length > 4) {
      k.style.width = (100 / 15 * item[4] / 30) + '%';
    }
    if (item.length > 2) {
      var keycodes = [item[2], item[3]];
    } else {
      var keycodes = [item[0].charCodeAt(0), item[1].charCodeAt(0)];
    }
    var labels = [item[0], item[1]];
    k.onclick = function() {
      if (item[0] === SHIFT[0]) {
        shift = 1 - shift;
        UpdateKeys();
      } else {
        context.inbuffer.push(keycodes[shift]);
      }
    };
    k.update = function() {
      if (item[0] === SHIFT[0]) {
        k.style.backgroundImage = shift ? KEY_GOLD : KEY_COLOR;
      }
      k.innerHTML = labels[shift];
    };
    context.keyboard.appendChild(k);
    keys.push(k);
  }
  for (var i = 0; i < KEYMAP.length; ++i) {
    var item = KEYMAP[i];
    AddKey(item);
  }
  UpdateKeys();
  if (/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent)) {
    context.screen.appendChild(context.keyboard);
  }

  context.text_fraction = 1667;
  context.min_text_portion = 120;
  context.mode = 1;
  function setMode(mode) {
    if (context.mode === mode) {
      return ;
    }
    if (mode) {
      context.filler.style.display = '';
      context.canvas.style.display = '';
    } else {
      context.filler.style.display = 'none';
      context.canvas.style.display = 'none';
    }
    context.mode = mode;
  }
  context.setMode = setMode;
  function Resize() {
    var width = window.innerWidth;
    var theight = Math.max(context.min_text_portion,
                           Math.floor(window.innerHeight *
                                      context.min_text_portion / 10000));
    var height = window.innerHeight - theight;
    if (width === context.width && height === context.height) {
      return;
    }
    context.canvas.style.width = width + 'px';
    context.canvas.style.height = height + 'px';
    if (context.text_fraction == 0 &&
        context.min_text_portion == 0) {
      context.filler.style.width = '1px';
      context.filler.style.height = '0px';
    } else {
      context.filler.style.width = '1px';
      context.filler.style.height = height + 'px';
    }
    context.width = width;
    context.height = height;
  }
  context.Resize = Resize;
  function Clear() {
    Resize();
    context.ctx.fillStyle = '#000';
    context.ctx.fillRect(0, 0, context.canvas.width, context.canvas.height);
  }
  context.Clear = Clear;
  window.onresize = function(e) {
    Resize();
  };
  function KeyPress(e) {
    context.inbuffer.push(e.keyCode);
    e.preventDefault();
    return false;
  }
  window.onkeypress = KeyPress;
  function KeyDown(e) {
    if (e.keyCode == 8) {
      context.inbuffer.push(e.keyCode);
      e.preventDefault();
      return false;
    }
  }
  window.onkeydown = KeyDown;
  context.Update = function(active) {
    var cursor = String.fromCharCode(0x2592);
    context.terminal.innerText = context.outbuffer + cursor;
  };
  setMode(0);
  context.Clear();
}
~ jseval

r|
(function(sp) {
  var n = i32[sp>>2]; sp -= 4;
  var a = i32[sp>>2]; sp -= 4;
  if (globalObj.write) {
    var text = GetString(a, n);
    write(text);
  } else {
    for (var i = 0; i < n; ++i) {
      var ch = u8[a + i];
      if (ch == 12) {
        context.outbuffer = '';
      } else if (ch == 8) {
        context.outbuffer = context.outbuffer.slice(0, -1);
      } else if (ch == 13) {
      } else {
        context.outbuffer += String.fromCharCode(ch);
      }
    }
    context.Update();
    window.scrollTo(0, document.body.scrollHeight);
  }
  return sp;
})
| 2 jseval!
: web-type ( a n -- ) 2 call yield ;
' web-type is type

r|
(function(sp) {
  if (globalObj.readline && !context.inbuffer.length) {
    var line = readline();
    for (var i = 0; i < line.length; ++i) {
      context.inbuffer.push(line.charCodeAt(i));
    }
    context.inbuffer.push(13);
  }
  if (context.inbuffer.length) {
    sp += 4; i32[sp>>2] = context.inbuffer.shift();
  } else {
    sp += 4; i32[sp>>2] = 0;
  }
  return sp;
})
| 3 jseval!
: web-key ( -- n ) begin yield 3 call dup if exit then drop again ;
' web-key is key

r|
(function(sp) {
  if (globalObj.readline) {
    sp += 4; i32[sp>>2] = -1;
    return sp;
  }
  sp += 4; i32[sp>>2] = context.inbuffer.length ? -1 : 0;
  return sp;
})
| 4 jseval!
: web-key? ( -- f ) yield 4 call ;
' web-key? is key?

r|
(function(sp) {
  var val = i32[sp>>2]; sp -= 4;
  if (globalObj.quit) {
    quit(val);
  } else {
    Init();
  }
  return sp;
})
| 5 jseval!
: terminate ( n -- ) 5 call ;

r|
(function(sp) {
  if (globalObj.write) {
    sp += 4; i32[sp>>2] = 0;  // Disable echo.
  } else {
    sp += 4; i32[sp>>2] = -1;  // Enable echo.
  }
  return sp;
})
| 6 jseval! 6 call echo !

r|
(function(sp) {
  var mode = i32[sp>>2]; sp -= 4;
  if (globalObj.write) {
    return sp;
  }
  context.setMode(mode);
  return sp;
})
| 7 jseval!

r|
(function(sp) {
  var c = i32[sp>>2]; sp -= 4;
  var h = i32[sp>>2]; sp -= 4;
  var w = i32[sp>>2]; sp -= 4;
  var y = i32[sp>>2]; sp -= 4;
  var x = i32[sp>>2]; sp -= 4;
  if (globalObj.write) {
    return sp;
  }
  function HexDig(n) {
    return ('0' + n.toString(16)).slice(-2);
  }
  context.ctx.fillStyle = '#' + HexDig((c >> 16) & 0xff) +
                                HexDig((c >> 8) & 0xff) +
                                HexDig(c & 0xff);
  context.ctx.fillRect(x, y, w, h);
  return sp;
})
| 8 jseval!

r|
(function(sp) {
  var h = i32[sp>>2]; sp -= 4;
  var w = i32[sp>>2]; sp -= 4;
  if (globalObj.write) {
    return sp;
  }
  context.canvas.width = w;
  context.canvas.height = h;
  return sp;
})
| 9 jseval!

r|
(function(sp) {
  if (globalObj.write) {
    sp += 4; i32[sp>>2] = 1;
    sp += 4; i32[sp>>2] = 1;
    return sp;
  }
  sp += 4; i32[sp>>2] = context.width;
  sp += 4; i32[sp>>2] = context.height;
  return sp;
})
| 10 jseval!

r|
(function(sp) {
  var mp = i32[sp>>2]; sp -= 4;
  var tf = i32[sp>>2]; sp -= 4;
  if (globalObj.write) {
    return sp;
  }
  context.text_fraction = tf;
  context.min_text_portion = mp;
  context.Resize();
  return sp;
})
| 11 jseval!

forth definitions web

: bye   0 terminate ;
: page   12 emit ;
: gr   1 7 call ;
: text   0 7 call ;
$ffffff value color
: box ( x y w h -- ) color 8 call ;
: window ( w h -- ) 9 call ;
: viewport@ ( -- w h ) 10 call ;
: show-text ( f -- ) if 1667 120 else 0 0 then 11 call ;

forth definitions