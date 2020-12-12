#!/usr/bin/perl
#
# res2html.pl
# -----------
#
# result.unl Datei von resp in HTML-Format umwandeln.
# Peter Rosendahl / 20.01.02 / perl v5.6.1 (built for cygwin-multi)
# $Id: res2html.pl,v 1.3 2002/03/02 02:13:51 rosendahl Exp $
#
#-------------------------------------------------------
# AUFRUF:
#         ----->  res2html.pl results.unl
#-------------------------------------------------------
#
# DEFINITION FÜR DAS VERLANGTE UNLOAD FORMAT:
#
# "result.unl" muss folgerndermassen aufgebaut sein:
#
# <ENTRY 0>|<ENTRY 1>|<ENTRY 2>| .... |<ENTRY n>\n
#
# Die Bedeutung von <ENTRY X> kann man folgernder Auflistung entnehmen
# (X heisst: Eintrag muss vorhanden sein; alle anderen sind optional!):
#

$EPD_NAME      = 0;  # Name der Testsuite (X)
$COMPUTER      = 1;  # Verwendeter Computer
$ENG_NAME      = 2;  # Name der Engine (X)
$ENG_VERSION   = 3;  # Version der Engine
$ENG_HASH      = 4;  # Groesse des Hash
$ENG_NOTES     = 5;  # Zusaetzliche Informationen zur Konfiguration
$DATE          = 6;  # Datum des Tests 
$N_POSITIONS   = 7;  # Anzahl der Testpositionen (X)
$N_SOLVED      = 8;  # Davon geloest? (X)
$MAX_TIME      = 9;  # Maximale Zeit pro Position (X)
$AVER_DEPTH    =10;  # Durchschnittliche Suchtiefe
$AVER_NODES    =11;  # Durchschnittliche Suchbaumgroesse
$TIME_USED     =12;  # Insgesamt verwendete Zeit
$N_MATES       =13;  # Gefundene Mattpositionen
$MOVEORDERING  =14;  # Anzahl Beta-Cutoffs beim ersten Versuch
$RES_NAME      =15 ; # Datei mit Einzel-Resultaten


while (<>)
{

   
   # get data
   chop;
   @a = split(/\|/);

   # "must" fields available?
   if (!$a[$EPD_NAME] || !$a[$ENG_NAME] || !$a[$N_POSITIONS] || 
       !$a[$N_SOLVED] || !$a[$MAX_TIME])
   {
      next;
   }
   
   # Unique name for test-suite: 'epd-filename' + '(' + 'max_time' + ')'
   $test_name = "$a[$EPD_NAME] ($a[$MAX_TIME] secs)";
   print "Found $test_name ... ";

   # add to results-hash
   $ref_result = [ @a ];
   if ($results{$test_name})
   {
      print "adding result.\n";
      $ref_result_list = $results{$test_name};
      push(@$ref_result_list, $ref_result);
   }
   else
   {
      print "new result.\n";
      $results{$test_name} = [ $ref_result ];
   }

}


#to_ascii();

print "\n\n";
to_html();

gen_index();

# ------------------------------------------------------------------
# to_ascii ... output in  ASCII - format
# ------------------------------------------------------------------
sub to_ascii
{
   print "---------------------------------------\n";
   foreach $test_name (keys (%results))
   {
      printf "\n\n";
      printf "--------------------------------------------\n";
      printf " TESTSUITE:  $test_name\n";
      printf "--------------------------------------------\n";

      $ref_result_list = $results{$test_name};
      print "#Entries: " . @$ref_result_list . "\n\n";

      printf("%-20s | %-8s | %-10s | %5s\n", "ENGINE","DATE","RESULT","AV.DEPTH");
      printf "--------------------------------------------------------\n";
      foreach $ref_result (@$ref_result_list)
      {
         printf ("%-20s | %-8s | %-10s | %5.1f\n", "$$ref_result[3] $$ref_result[4]",
                 $$ref_result[6],
                 "$$ref_result[8] / $$ref_result[7]", $$ref_result[10]
         );
      }
   }
}

# ------------------------------------------------------------------
# to_html ... Output of the  test results in HTML format
# ------------------------------------------------------------------

sub to_html()
{
   foreach $test_name (keys (%results))
   {
      print "Generating HTML: $test_name ... \n";
      open(OUT,">${test_name}.html") || die "Oops....!";
      
      # ---------------
      #  html - head
      # ---------------
      print OUT "<html>\n<head>\n";
      print OUT "<title>${test_name}</title>\n";
      print OUT "</head>\n";

      # ---------------
      #  html - body
      # ---------------
      print OUT "<body>\n";

      # titel
      print OUT "<h2>Results for <i>${test_name}</i></h2>\n";

      # Referenz auf Result-Satz holen
      $ref_result_list = $results{$test_name};

      #print OUT "<small><i> #Results found: " . @$ref_result_list . "</i></small>\n";
      print OUT "<p>";

      # --------------
      #  table head
      # --------------
      print OUT "<table border=1 cellspacing=0 cellpadding=3>\n";

      print OUT "<TR>\n";
      printTH ("<b>Engine/Version</b>");
      printTH ("<b>Date</b>");
      printTH ("<b>Result</b>");
      printTH ("<b>#Mates</b>");
      printTH ("<b>Total Time</b>");
      printTH ("<b>Aver. Nodes</b>");
      printTH ("<b>Aver. Depth</b>");
      printTH ("<b>Moveordering</b>");
      print OUT "</TR>\n";

      foreach $ref_result (@$ref_result_list)
      {
         print OUT "<TR>\n";
         
         # program version (configuration)
         $eng =  "$$ref_result[$ENG_NAME] $$ref_result[$ENG_VERSION]";
         if ($$ref_result[$ENG_HASH])
         {
            $eng = $eng . " ($$ref_result[$ENG_HASH] MB)";
         }
         
         # if there are individual result for this test, also generate
         # a html page for that
         $ind_res = 0;
         if ($$ref_result[$RES_NAME])
         {
            $ind_res = gen_indresults($ref_result, $test_name);
         }
         
         if (!$ind_res)
         {
            printTD ($eng);
         }
         else
         {
            print OUT "<td>";
            print OUT "<a href=\"$$ref_result[$RES_NAME].html\">\n";
            print OUT $eng;
            print OUT "</a></td>\n";
         }
         # date
         printTD ("$$ref_result[$DATE]");
         # result
         printTD ("$$ref_result[$N_SOLVED]/$$ref_result[$N_POSITIONS]"," align=\"right\"");
         # number of mates
         printTD ("$$ref_result[$N_MATES]"," align=\"right\"");
         # total time
         printTD ("$$ref_result[$TIME_USED]"," align=\"right\"");
         # average size of search tree
         printTD ("$$ref_result[$AVER_NODES]"," align=\"right\"");
         # average search depth
         printTD ("$$ref_result[$AVER_DEPTH]"," align=\"right\"");
         # betacutoffs on first move
         printTD ("$$ref_result[$MOVEORDERING]%"," align=\"right\"");
         print OUT "</TR>\n";
         
      }

      print OUT "</table>";
      # ---------------
      #  html - close
      # ---------------
      print OUT "<p>";
      print OUT "<pre>";
      print OUT "  Created with <i>res2html.pl</i>";
      print OUT "</pre>\n";
      print OUT "</body>\n</html>\n";

      close OUT;
      print "OK.\n";
   }
}


sub printTD()
{
   print OUT "<td$_[1]>$_[0]</td>";
}
sub printTH()
{
   print OUT "<th>$_[0]</th>";
}

#---------------------------------------------------------
# Generate indiviual statistics for _one_ testrun
#---------------------------------------------------------
sub gen_indresults()
{
   $ref_result = $_[0];
   $test_name  = $_[1];

   
   print "   Generating individual results for $$ref_result[$RES_NAME].html ...";
   if (!open(IND_OUT,">$$ref_result[$RES_NAME].html"))
   {
      print "\n   Error! Couldn't open file.\n";
      return 0;
   }

   # ---------------
   #  html - head
   # ---------------
   print IND_OUT "<html>\n<head>\n";
   print IND_OUT "<title>Individual results for ${test_name}</title>\n";
   print IND_OUT "</head>\n";

   # ---------------
   #  html - body
   # ---------------
   print IND_OUT "<body>\n";

   # titel
   print IND_OUT "<h2>Individual results for <i>${test_name}</i></h2>\n";
   
   # information about testrun
   print IND_OUT "<pre>\n";

   print IND_OUT "Name of epd file  : $$ref_result[$EPD_NAME]\n";
   print IND_OUT "System            : $$ref_result[$COMPUTER]\n";
   print IND_OUT "Engine            : $$ref_result[$ENG_NAME]\n";
   print IND_OUT "Engine Version    : $$ref_result[$ENG_VERSION]\n";
   print IND_OUT "Size of Hash      : $$ref_result[$ENG_HASH] MB\n";
   print IND_OUT "Notes             : $$ref_result[$ENG_NOTES]\n";
   print IND_OUT "Date              : $$ref_result[$DATE]\n";
   print IND_OUT "#Position         : $$ref_result[$N_POSITIONS]\n";
   print IND_OUT "#Solved           : $$ref_result[$N_SOLVED]\n";
   print IND_OUT "Max.time/position : $$ref_result[$MAX_TIME] sec.\n";
   print IND_OUT "Average depth     : $$ref_result[$AVER_DEPTH]\n";
   print IND_OUT "Average nodes     : $$ref_result[$AVER_NODES]\n";
   print IND_OUT "Time used         : $$ref_result[$TIME_USED]\n";
   print IND_OUT "#Mates found      : $$ref_result[$N_MATES]\n";
   print IND_OUT "Moveordering      : $$ref_result[$MOVEORDERING]\%\n";
   
   print IND_OUT "</pre>\n";

   print IND_OUT "<h3>Results</h3>\n";

   # ---------------------------------------------
   #  Table with solution time for every position
   # ---------------------------------------------
   if (!open(IND_IN,"$$ref_result[$RES_NAME]"))
   {
      print "\n   Error! Couldn't open $$ref_result[$RES_NAME]\n";
      return 0;
   }

   print IND_OUT "<table border=1 cellspacing=0 cellpadding=3>\n";

   $npos = 0;   # Current position

   while (<IND_IN>)
   {
      chop;
      $sol_time = $_;
      
      # new row
      if ($npos % 10 == 0)
      {
         if ($npos)  # not the first row
         {
            print IND_OUT "</TR>\n";
         }
         print IND_OUT "<TR>\n";
         print IND_OUT "<TD><b>\n";
         printf IND_OUT "Pos. %i - %i\n", $npos+1,$npos+10;
         print IND_OUT "</b></TD>\n";
      }
      ++$npos;

      print IND_OUT "<TD>\n";

      if ($sol_time == -1)    # not solved?
      {
         print IND_OUT "<i>nope</i>";
      }
      else
      {
         printf IND_OUT "%6.3f\n", $sol_time/1000.0;
      }
      
      print IND_OUT "</TD>\n";
   }

   print IND_OUT "</TR>\n";   # Last row
   
   print IND_OUT "</table>\n";

   # ---------------
   #  html - close
   # ---------------
   print IND_OUT "<p>";
   print IND_OUT "<pre>";
   print IND_OUT "  Created with <i>res2html.pl</i>";
   print IND_OUT "</pre>\n";
   print IND_OUT "</body>\n</html>\n";
   close IND_OUT;
   print "OK.\n";

   return 1;
}

#---------------------------------------------------------
# gen_index() ... generate index.html for the testresults
#---------------------------------------------------------
sub gen_index()
{
   open(IDX_OUT,">index.html") || die "Can't create index.html!";

   # ---------------
   #  html - head
   # ---------------
   print IDX_OUT "<html>\n<head>\n";
   print IDX_OUT "<title>Testresults</title>\n";
   print IDX_OUT "</head>\n";
   print IDX_OUT "<body>\n";
   # titel
   print IDX_OUT "<h2>Testresults</i></h2>\n";

   print IDX_OUT "<h3>List of Testsuites</h3>\n";
   print IDX_OUT "Results for the following test suites are available: <p>\n";

   @names = sort keys(%results);

   foreach $test_name (@names)
   {
      # link to testsuite results
      print IDX_OUT "<a href=\"${test_name}.html\">\n";
      print IDX_OUT "$test_name";
      print IDX_OUT "</a>\n";
      $ref_res_array = $results{$test_name};
      $n_entries = @$ref_res_array;
      print IDX_OUT " (with $n_entries entries)\n";
      print IDX_OUT "<br>\n";
   }   

   # ---------------
   #  html - close
   # ---------------
   print IDX_OUT "<p>";
   print IDX_OUT "<pre>";
   print IDX_OUT "  Created with <i>res2html.pl</i>";
   print IDX_OUT "</pre>\n";
   print IDX_OUTUT "</body>\n</html>\n";
   close IDX_OUT;

}
