# kmerfreq
kmerfreq count K-mer (with size K) frequency from the input sequence data, typically sequencing reads data, and refe rence genome data is also applicable. Kmerferq output file can be used as input file of either version of GCE (Genomic character estimator).

## Includes

* [klib](https://github.com/attractivechaos/klib)
* [lfqueue](https://github.com/Taymindis/lfqueue)

## etc

For Valgrind on macOS Mojave (tested on 10.14.6), refer to <https://stackoverflow.com/questions/52732036/how-to-install-valgrind-on-macos-mojave10-14-with-homebrew>. 
To install it, use `brew install --HEAD https://raw.githubusercontent.com/sowson/valgrind/master/valgrind.rb` or `brew install --HEAD https://raw.githubusercontent.com/LouisBrunner/valgrind-macos/master/valgrind.rb`.
