#! /usr/bin/env ruby

(1..9).each do |n|
  print <<"EOF"
[example0#{n}.c](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/input/example0#{n}.c) | [example0#{n}.txt](https://github.com/jhucompilers/fall2022-tests/blob/main/assign04/example_highlevel_code/example0#{n}.txt)
EOF
end
