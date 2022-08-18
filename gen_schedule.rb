#! /usr/bin/env ruby

require 'csv'

FRONT_STUFF = <<'EOF1'
---
layout: default
title: "Schedule"
category: "schedule"
---

This page lists lecture topics, readings, and exam dates.  It also lists assignment due dates.

The links to slides are provided for reference.  In general, there is no
guarantee that they will be posted before class, or that their content
will not change.

Readings:

* EaC is Cooper and Torczon, [Engineering a Compiler (2nd
  Ed.)](https://www.elsevier.com/books/engineering-a-compiler/cooper/978-0-12-088478-0)
* F&amp;B is Levine, [Flex and Bison](https://www.oreilly.com/library/view/flex-bison/9780596805418/)

*Note*: The schedule will become more concrete as the semester
progresses. Expect it to be updated frequently.  Tentative topics,
readings, assignments, etc. are marked <span class="tentative">in
a lighter italic font</span>: expect that these could change.

This page lists topics, readings, and has links to lecture slides.
It also lists assignment due dates.  Dates <span class="tentative">in
gray</span> are tentative.

Date&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | Topic/Slides | Reading | Assignment
------------------ | ------------ | ------- | ----------
EOF1

print FRONT_STUFF

first = true
CSV.foreach('schedule.csv') do |row|
  if first
    first = false
  else
    # Date,Topic,Slides,"Example Code",Reading,Assignment
    while row.length < 6
      row.push('')
    end

    row = row.map {|x| x.nil? ? '' : x}

    date, topic, slides, example_code, reading, assignment = row

    #puts date

    print date

    if slides != ''
      print " | [#{topic}](#{slides})"
    else
      print " | #{topic}"
      #print ": [slides](#{slides})"
    end

    if example_code != ''
      print ", [#{example_code} (example code)](lectures/#{example_code})"
    end

    print " | #{reading}"

    puts " | #{assignment}"
  end
end
