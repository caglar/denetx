#!/usr/bin/perl -ws

use warnings;
use strict ;
use Net::Flow qw(decode);
use Net::Flow::Ie qw(decode addie);
#use Ie qw(iedecode addie);
use IO::Socket::INET;

my $receive_port = 8991;
my $packet = undef;
my $TemplateArrayRef = undef;
my $sock = IO::Socket::INET->new( LocalPort =>$receive_port, Proto => 'udp');
my $ieRef = Net::Flow::Ie::addie();

while ($sock->recv($packet,1548)) {

    my ($HeaderHashRef,$FlowArrayRef,$ErrorsArrayRef)=() ;

    ( $HeaderHashRef,
        $TemplateArrayRef,
        $FlowArrayRef,
        $ErrorsArrayRef)
    = Net::Flow::decode(
        \$packet,
    $TemplateArrayRef
) ;

grep{ print "$_\n" }@{$ErrorsArrayRef} if( @{$ErrorsArrayRef} ) ;

print "\n- Header Information -\n" ;
foreach my $Key ( sort keys %{$HeaderHashRef} ){
    printf " %s = %3d\n",$Key,$HeaderHashRef->{$Key} ;
}

foreach my $TemplateRef ( @{$TemplateArrayRef} ){
    print "\n-- Template Information --\n" ;

    foreach my $TempKey ( sort {$a <=> $b} keys %{$TemplateRef} ){
        if( $TempKey eq "Template" ){

            printf "  %s = \n",$TempKey ;

            foreach my $Ref ( @{$$TemplateRef{Template}}  ){

                foreach my $Key ( keys %{$Ref} ){

                    printf "   %s=%-3d Name=%-30s Type=%-10s",
                    $Key, $$Ref{$Key}, $$ieRef{$$Ref{$Key}}->{Name},
                    $$ieRef{$$Ref{$Key}}->{Type} if $Key eq "Id" ;

                    printf "   %s=%-3d", $Key, $$Ref{$Key} if $Key eq "Length" ;

                }

                print "\n" ;

            }

        }else{

            printf "  %s = %s\n", $TempKey, $$TemplateRef{$TempKey} ;

        }

    }

}

foreach my $FlowRef ( @{$FlowArrayRef} ){
    print "\n-- Flow Information --\n" ;

    foreach my $Id ( sort {$a <=> $b} keys %{$FlowRef} ){

        if( $Id eq "SetId" ){

            print "  $Id=$$FlowRef{$Id}\n" if defined $$FlowRef{$Id} ;

        }else{

            printf "  Id=%-3d Name=%-30s Value=%s\n",
            $Id, Net::Flow::Ie::decode($Id,$$FlowRef{$Id}) ;

        }

    }

}
}
1;
