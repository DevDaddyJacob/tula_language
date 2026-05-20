-- ============================================================
--  lua_syntax_demo.lua
--  A single-file tour of standard Lua syntax (Lua 5.4)
--  Run with:  lua lua_syntax_demo.lua
-- ============================================================


-- ── 1. COMMENTS ─────────────────────────────────────────────

-- Single-line comment

--[[
  Multi-line comment.
  Everything between the long brackets is ignored.
--]]


-- ── 2. PRIMITIVE TYPES & LITERALS ───────────────────────────

local n_integer  = 42
local n_float    = 3.14
local n_hex      = 0xFF          -- 255
local n_sci      = 1.5e3         -- 1500.0
local b_true     = true
local b_false    = false
local nothing    = nil
local s_single   = 'Hello'
local s_double   = "World"
local s_long     = [[
  Long strings keep
  newlines verbatim.
]]

print("--- Types & Literals ---")
print(type(n_integer), type(b_true), type(nothing), type(s_single))


-- ── 3. VARIABLES: LOCAL vs GLOBAL ───────────────────────────

globalVar = "I am global"          -- global (avoid in real code)
local localVar = "I am local"      -- local to this block

do
  local scoped = "only inside do…end"
  -- scoped is visible here
end
-- scoped is nil here


-- ── 4. OPERATORS ────────────────────────────────────────────

print("\n--- Operators ---")

-- Arithmetic
print(10 + 3, 10 - 3, 10 * 3, 10 / 3)   -- / always returns float
print(10 // 3)                            -- floor division → 3
print(10 % 3)                             -- modulo         → 1
print(2 ^ 10)                             -- exponentiation → 1024.0
print(-n_integer)                         -- unary minus

-- Relational  (always return boolean)
print(1 == 1, 1 ~= 2, 3 < 4, 4 > 3, 3 <= 3, 4 >= 5)

-- Logical
print(true and false)     -- false
print(true or  false)     -- true
print(not true)           -- false
-- Short-circuit: returns an operand, not necessarily a boolean
print(nil or "default")   -- "default"
print("a" and "b")        -- "b"

-- String concatenation & length
local greeting = "Hello" .. ", " .. "Lua!"
print(greeting, #greeting)   -- length operator

-- Bitwise (Lua 5.3+)
print(0xFF & 0x0F)   -- AND  → 15
print(0xF0 | 0x0F)   -- OR   → 255
print(0xFF ~ 0x0F)   -- XOR  → 240
print(~0)            -- NOT  → -1
print(1 << 4)        -- left shift  → 16
print(256 >> 4)      -- right shift → 16

-- Integer division and the new integer subtype (Lua 5.3+)
local i = 10 // 1    -- integer
local f = 10.0 // 1  -- float
print(math.type(i), math.type(f))


-- ── 5. STRINGS – STANDARD LIBRARY ───────────────────────────

print("\n--- Strings ---")
local s = "Lua is great"
print(string.upper(s))
print(string.lower(s))
print(string.len(s))
print(string.sub(s, 1, 3))           -- "Lua"
print(string.rep("ab", 3, "-"))      -- "ab-ab-ab"
print(string.reverse(s))
print(string.format("Pi ≈ %.4f", math.pi))
print(string.find(s, "great"))       -- 8  12
print(string.gsub(s, "great", "awesome"))

-- Method-style calls (syntactic sugar via __index on string metatable)
print(s:upper())
print(("hello"):rep(2, " | "))

-- String.byte / char
print(string.byte("A"))             -- 65
print(string.char(65, 66, 67))      -- "ABC"

-- Pattern matching
local date = "2026-05-20"
local y, m, d = date:match("(%d+)-(%d+)-(%d+)")
print(y, m, d)

for word in ("one two three"):gmatch("%a+") do
  io.write(word .. " ")
end
print()


-- ── 6. CONTROL FLOW ─────────────────────────────────────────

print("\n--- Control Flow ---")

-- if / elseif / else
local x = 7
if x < 0 then
  print("negative")
elseif x == 0 then
  print("zero")
else
  print("positive: " .. x)
end

-- while
local i = 1
while i <= 3 do
  io.write(i .. " ")
  i = i + 1
end
print()

-- repeat … until  (condition checked AFTER body)
local j = 1
repeat
  io.write(j .. " ")
  j = j + 1
until j > 3
print()

-- Numeric for:  var = start, limit [, step]
for k = 1, 5 do io.write(k .. " ") end
print()

for k = 10, 1, -3 do io.write(k .. " ") end
print()

-- Generic for with ipairs (ordered, integer keys)
local fruits = {"apple", "banana", "cherry"}
for idx, val in ipairs(fruits) do
  io.write(idx .. "=" .. val .. " ")
end
print()

-- Generic for with pairs (all keys, unordered)
local scores = {alice = 95, bob = 87, carol = 92}
for name, score in pairs(scores) do
  io.write(name .. ":" .. score .. " ")
end
print()

-- break  (exits the innermost loop)
for k = 1, 10 do
  if k == 4 then break end
  io.write(k .. " ")
end
print()

-- goto  (Lua 5.2+)
for k = 1, 5 do
  if k == 3 then goto continue end
  io.write(k .. " ")
  ::continue::
end
print()


-- ── 7. FUNCTIONS ────────────────────────────────────────────

print("\n--- Functions ---")

-- Basic declaration
local function add(a, b)
  return a + b
end
print(add(3, 4))

-- Functions are first-class values
local multiply = function(a, b) return a * b end
print(multiply(3, 4))

-- Multiple return values
local function minmax(t)
  local lo, hi = t[1], t[1]
  for _, v in ipairs(t) do
    if v < lo then lo = v end
    if v > hi then hi = v end
  end
  return lo, hi
end
local lo, hi = minmax({5, 2, 8, 1, 9})
print("min=" .. lo .. "  max=" .. hi)

-- Variadic functions  (...)
local function sum(...)
  local args = {...}
  local total = 0
  for _, v in ipairs(args) do total = total + v end
  return total
end
print(sum(1, 2, 3, 4, 5))
print(select("#", 10, 20, 30))   -- count of extra args = 3
print(select(2,  10, 20, 30))    -- 20  30

-- Closures
local function counter(start)
  local count = start or 0
  return function()
    count = count + 1
    return count
  end
end
local c = counter(10)
print(c(), c(), c())   -- 11  12  13

-- Tail calls (Lua optimises proper tail calls)
local function fact_tail(n, acc)
  acc = acc or 1
  if n <= 1 then return acc end
  return fact_tail(n - 1, n * acc)
end
print(fact_tail(10))   -- 3628800

-- Named method syntax sugar  (colon operator)
local obj = { value = 42 }
function obj:getValue()   -- same as obj.getValue(self)
  return self.value
end
print(obj:getValue())


-- ── 8. TABLES ───────────────────────────────────────────────

print("\n--- Tables ---")

-- Array-style (1-based)
local arr = {10, 20, 30, 40, 50}
print(#arr)                         -- 5
table.insert(arr, 60)
table.remove(arr, 1)                -- removes first element
table.sort(arr, function(a, b) return a > b end)
print(table.concat(arr, ", "))

-- Dictionary-style
local person = {
  name  = "Lua",
  born  = 1993,
  typed = true,
}
person.author = "PUC-Rio"           -- add field
person["born"] = nil                -- remove field
for k, v in pairs(person) do
  print("  " .. tostring(k) .. " = " .. tostring(v))
end

-- Mixed table
local mixed = {"a", "b", x = 1, y = 2, "c"}
print(#mixed)    -- 3  (only the sequence part)

-- Nested tables
local matrix = {{1,2},{3,4},{5,6}}
print(matrix[2][1])   -- 3

-- table.move (Lua 5.3+)
local src = {1,2,3,4,5}
local dst = {}
table.move(src, 2, 4, 1, dst)   -- copy src[2..4] → dst[1..3]
print(table.concat(dst, ","))    -- 2,3,4

-- table.unpack
print(table.unpack({100, 200, 300}))


-- ── 9. METATABLES & METAMETHODS ─────────────────────────────

print("\n--- Metatables ---")

local Vector = {}
Vector.__index = Vector

function Vector.new(x, y)
  return setmetatable({x=x, y=y}, Vector)
end

function Vector:__tostring()
  return string.format("Vector(%g, %g)", self.x, self.y)
end

function Vector:__add(other)
  return Vector.new(self.x + other.x, self.y + other.y)
end

function Vector:__sub(other)
  return Vector.new(self.x - other.x, self.y - other.y)
end

function Vector:__mul(scalar)
  return Vector.new(self.x * scalar, self.y * scalar)
end

function Vector:__eq(other)
  return self.x == other.x and self.y == other.y
end

function Vector:__len()
  return math.sqrt(self.x^2 + self.y^2)
end

function Vector:__unm()   -- unary minus
  return Vector.new(-self.x, -self.y)
end

local v1 = Vector.new(1, 2)
local v2 = Vector.new(3, 4)
print(tostring(v1))
print(tostring(v1 + v2))
print(tostring(v1 - v2))
print(tostring(v1 * 3))
print(#v2)              -- sqrt(9+16) = 5.0
print(v1 == Vector.new(1,2))

-- __index for inheritance / default values
local Defaults = {color = "red", size = 10}
local widget = setmetatable({size = 20}, {__index = Defaults})
print(widget.size, widget.color)    -- 20  red  (size overridden)

-- __newindex to intercept writes
local readonly = setmetatable({}, {
  __newindex = function(_, k, _)
    error("attempt to write read-only field: " .. k)
  end,
  __index = {pi = math.pi}
})
print(readonly.pi)
-- readonly.pi = 3  -- would error

-- __call makes a table callable
local callable = setmetatable({}, {
  __call = function(self, a, b) return a + b end
})
print(callable(7, 8))   -- 15

-- rawget / rawset (bypass metamethods)
rawset(widget, "color", "blue")
print(rawget(widget, "color"))


-- ── 10. OBJECT-ORIENTED PROGRAMMING ─────────────────────────

print("\n--- OOP ---")

-- Base class
local Animal = {}
Animal.__index = Animal

function Animal.new(name, sound)
  return setmetatable({name=name, sound=sound}, Animal)
end

function Animal:speak()
  print(self.name .. " says " .. self.sound)
end

function Animal:__tostring()
  return "Animal(" .. self.name .. ")"
end

-- Derived class (single inheritance)
local Dog = setmetatable({}, {__index = Animal})
Dog.__index = Dog

function Dog.new(name)
  local self = Animal.new(name, "Woof")
  return setmetatable(self, Dog)
end

function Dog:fetch(item)
  print(self.name .. " fetches the " .. item)
end

local d = Dog.new("Rex")
d:speak()          -- inherited
d:fetch("ball")    -- own method


-- ── 11. ERROR HANDLING ───────────────────────────────────────

print("\n--- Error Handling ---")

-- pcall: protected call; returns true/false + result/message
local ok, err = pcall(function()
  error("something went wrong")
end)
print(ok, err)

-- error with table (structured errors)
local ok2, err2 = pcall(function()
  error({code = 404, msg = "not found"}, 0)
end)
print(ok2, type(err2), err2.code, err2.msg)

-- xpcall: like pcall but with a message handler
local function msgh(e)
  return "CAUGHT: " .. tostring(e)
end
local ok3, msg3 = xpcall(function()
  error("oops")
end, msgh)
print(ok3, msg3)

-- assert
local function divide(a, b)
  assert(b ~= 0, "division by zero")
  return a / b
end
print(divide(10, 2))
local ok4, err4 = pcall(divide, 10, 0)
print(ok4, err4)


-- ── 12. COROUTINES ───────────────────────────────────────────

print("\n--- Coroutines ---")

-- Producer / consumer with coroutines
local function producer()
  local items = {"alpha", "beta", "gamma"}
  for _, v in ipairs(items) do
    coroutine.yield(v)
  end
end

local co = coroutine.create(producer)

while true do
  local ok, val = coroutine.resume(co)
  if not ok or val == nil then break end
  print("received: " .. val)
end
print("coroutine status: " .. coroutine.status(co))  -- dead

-- coroutine.wrap (simpler iterator interface)
local gen = coroutine.wrap(function()
  for i = 1, 4 do coroutine.yield(i * i) end
end)
for _ = 1, 4 do io.write(gen() .. " ") end
print()

-- Two-way communication with yield
local co2 = coroutine.create(function(a)
  print("co2 got:", a)
  local b = coroutine.yield(a * 2)
  print("co2 got:", b)
  return b * 3
end)
local _, r1 = coroutine.resume(co2, 5)   -- start, send 5
print("main got:", r1)                    -- 10
local _, r2 = coroutine.resume(co2, 7)   -- send 7
print("main got:", r2)                    -- 21


-- ── 13. MODULES (require / package) ─────────────────────────

print("\n--- Modules ---")

-- Simulate a module returned as a table
local mymod = (function()
  local M = {}

  local secret = "hidden"    -- module-private

  function M.greet(name)
    return "Hello, " .. name .. "! [" .. secret .. "]"
  end

  function M.VERSION() return "1.0.0" end

  return M
end)()

print(mymod.greet("world"))
print(mymod.VERSION())

-- package.loaded allows inspecting / caching already-loaded modules
-- package.path controls where require() searches for .lua files
-- (actual require() calls need real files, so omitted here)


-- ── 14. STANDARD LIBRARIES (highlights) ─────────────────────

print("\n--- Standard Libraries ---")

-- math
print(math.floor(3.9), math.ceil(3.1))
print(math.abs(-5), math.sqrt(16))
print(math.max(1,5,3), math.min(1,5,3))
print(math.huge, -math.huge, math.pi)
math.randomseed(os.time())
print(math.random(), math.random(1,6))
print(math.tointeger(3.0))   -- 3  (Lua 5.3+)

-- os
print(os.date("%Y-%m-%d %H:%M:%S"))
local t1 = os.clock()        -- CPU time used
-- (some work)
local t2 = os.clock()
print(string.format("CPU time: %.6f s", t2 - t1))

-- io
io.write("io.write doesn't add a newline\n")

-- table (already used above; one more)
local t = {3,1,4,1,5,9,2,6}
table.sort(t)
print(table.concat(t, " "))

-- string.format verbs quick-reference
print(string.format("|%10s|%-10s|", "right", "left"))
print(string.format("|%05d|%+.2f|%e|", 42, 3.14, 12345.678))
print(string.format("|%q|", 'He said "hi"\nbye'))   -- quoted

-- utf8 library (Lua 5.3+)
local utf = "Héllo"
print(utf8.len(utf))          -- character count
for pos, cp in utf8.codes(utf) do
  io.write(string.format("U+%04X ", cp))
end
print()


-- ── 15. MISCELLANEOUS FEATURES ──────────────────────────────

print("\n--- Miscellaneous ---")

-- Multiple assignment
local a2, b2, c2 = 1, 2, 3
print(a2, b2, c2)
a2, b2 = b2, a2   -- swap
print(a2, b2)

-- Excess values are discarded; missing ones become nil
local p, q, r = 10, 20
print(p, q, r)    -- 10  20  nil

-- tostring / tonumber
print(tostring(true), tostring(nil), tostring(3.14))
print(tonumber("  42  "), tonumber("0xff"), tonumber("10", 2))

-- String coercion in arithmetic (Lua auto-converts numeric strings)
print("10" + 5)    -- 15   (string coerced to number)
print(10 .. "")    -- "10" (number coerced to string for ..)

-- ipairs vs pairs behaviour note
local sparse = {[1]="a", [3]="c"}   -- missing [2]
for i, v in ipairs(sparse) do io.write(i..v.." ") end
print("(ipairs stops at gap)")
for k, v in pairs(sparse) do io.write(k..v.." ") end
print("(pairs sees all)")

-- load / loadstring (compile & execute a string at runtime)
local fn, err5 = load("return 6 * 7")
if fn then print(fn()) else print(err5) end

-- Ternary idiom (no ternary operator in Lua)
local val = true
local result = val and "yes" or "no"
print(result)

-- Integer / float distinction (Lua 5.3+)
print(1, 1.0, 1 == 1.0)                    -- true (equal)
print(math.type(1), math.type(1.0))        -- integer  float

-- String method-call chaining via metatable __index
print(("  trim me  "):match("^%s*(.-)%s*$"))


-- ── 16. WEAK TABLES & THE GC ────────────────────────────────

print("\n--- Weak Tables ---")

-- Weak values: entries collected if value has no other reference
local cache = setmetatable({}, {__mode = "v"})
do
  local tmp = {data = "expensive object"}
  cache["key"] = tmp
  print(cache["key"].data)   -- "expensive object"
end
-- After GC tmp may be collected; cache["key"] becomes nil
collectgarbage()
print(cache["key"])          -- nil (probably)

-- Finalizers (__gc metamethod, Lua 5.4)
local finalized = setmetatable({}, {
  __gc = function(self)
    -- Called when object is about to be collected
    io.write("(finalizer ran) ")
  end
})
finalized = nil
collectgarbage()
print()


-- ── 17. LUA 5.4 SPECIFICS ───────────────────────────────────

print("\n--- Lua 5.4 Features ---")

-- to-be-closed variables  (<close> attribute)
-- They call their __close metamethod when going out of scope.
local Resource = {}
Resource.__index = Resource
Resource.__close = function(self)
  io.write("(resource '" .. self.name .. "' closed) ")
end
function Resource.new(name)
  return setmetatable({name=name}, Resource)
end

do
  local r <close> = Resource.new("file_handle")
  io.write("using resource  ")
  -- r.__close is called automatically here
end
print()

-- Integer subtype (already shown in §4; emphasized again)
print(type(1), type(1.0))        -- "number" "number"
print(math.type(1))              -- "integer"
print(math.type(1.0))            -- "float"

-- math.tointeger
print(math.tointeger(5.0))   -- 5
print(math.tointeger(5.5))   -- nil (not a whole float)

-- warn() function (Lua 5.4)
warn("this is a Lua warning – check stderr")

print("\n=== Demo complete ===")