##
# Test for 'gets', 'readline' and 'readlines'
#

puts "\nplease input some words and input 'Enter':"
assert("default 'gets'") do
  !gets.empty?
end

puts "\nplease input some words and input 'Ctrl+D':"
assert("nil passed 'gets'") do
  !gets(nil).empty?
end

puts "\nplease input some words and hit 'Enter' twice:"
assert("empty passed 'gets'") do
  !gets("").empty?
end

puts "\nplease input 'Ctrl+D' immediately:"
assert("'gets' returns nil") do
  gets.nil?
end

puts "\nplease input 'Ctrl+D' immediately:"
assert("readline") do
  begin
    readline
    false
  rescue EOFError
    true
  end
end

puts "\nplease input 'Ctrl+D' immediately:"
assert("readlines") do
  readlines.empty?
end

puts "\nplease input one or more lines and input 'Ctrl+D':"
assert("readlines") do
  lines = readlines
  lines.instance_of?(Array) && !lines.empty?
end
