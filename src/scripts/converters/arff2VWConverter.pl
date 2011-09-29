#!/usr/bin/perl -ws

#Convert the arff files to the vw format
use strict;
use warnings;

my $arff = chomp($1);
my $vw = $arff;
$vw =~ s/"arff"/"vw"/i;

open(ARFF, $arff) or die $;
open(VW, ">>$vw") or die $;

while (my $line = <ARFF>) {
    my $vwLine = ""
    if (not ($line =~ m/^(@|#)/)) {
        my @tokens = split(",", $line);
        my $className = chomp(pop @tokens);
        $vwLine = $className." | features ";
        for($i=0; $i<=scalar(@tokens); $i++) {
            $vwLine .= $tokens[i]." ";
        }
        $vwLine .= " const=0.01\n";
    }
}

close(ARFF);
close(VW);
1;
