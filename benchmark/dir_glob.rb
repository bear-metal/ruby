max = 100_000
max.times { Dir.glob("*.{rb,yml}") }
