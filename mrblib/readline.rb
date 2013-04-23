##
# Kernel
#
module Kernel
  $/ ||= "\n"

  def readline(rs=$/)
    begin
      line = ARGF.readline rs
    rescue NameError
      unless $stdin.nil? then
        line = $stdin.readline rs
      else
        line = gets rs
      end
    end
    if line.nil? then
      raise EOFError, "end of file reached"
    end
    line
  end

  def readlines(rs=$/)
    lines = []
    loop do
      line = gets rs
      if line.nil? then
        break
      end
	  lines << line
    end
    lines
  end

end
