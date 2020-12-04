# tspatch

Simple tool to work on a TS file
- parse and show some information
- make some changes

This tool does not follow any spec, it is just being implemented on demand, started in 2011.

Tool in the shape of one-source-code file, because it's my choice.
 
Compilation on Linux:
gcc -m32 tspatch.ts -o tspatch

Compilation on Windows, with mingw-w64:
gcc.exe tspatch.c -o tspatch

USAGE

usage: tspatch filename.ts [commands]
where:
  filename.ts is input transport stream file
  [commands]  is one or several out of

    selection commands
    -pn <pn>           select a program with program number
                       NOTE: some display and patching commands
                       require a program selection;
                       if none selected then the first program
                       found in PAT will be automatically selected
    -aud <pid>         select a audio PID
    -vid <pid>         select a video PID

    display commands
    -show-time         display stream date and time
    -show-br           display instant and average bitrates
    -show-pat          display the PAT
    -show-pmt          display the PMT of selected program
    -show-pid          display all detected PIDs
    -show-all-pid      display all PIDs of all packets
    -show-all-pat      display all PATs (limited interest)
    -show-sdt          display the SDT
    -show-ait <pid>    display the AIT identified by its PID
                       also select the AIT PID for -set-ait- commands below
    -show-dsmcc <pid>  display info about DSMCC

    file patching commands
    -rep-tsid <tsid>   replace transport stream id by new one
    -rep-pn <pn>       replace selected program number by new one
    -rep-pmt <pid>     replace selected program PMT PID by new one
    -rep-pcr <pid>     replace selected program PCR PID by new one
    -rep-aud <pid>     replace selected audio PID or the first audio PID
                       found in selected program PMT by new one
    -rep-vid <pid>     replace selected video PID or the first video PID
                       found in selected program PMT by new one
    -rep-es-pid <pid> <newpid> replace ES PID by new PID in PMT
    -rep-prov <name>   replace provider name in SDT
    -rep-time <time>   where <time> can be either either the 'now' magic word or a year number
    -rep-pid <pid> <newpid>          replace PID value
    -dup-pid <srcpid> <dstpid>       duplicate packets
    -ins-pid <pid> <unique-pid.ts>   insert packets of <unique-pid.ts> after each packet identified by PID
    -crush-pid <pid> <unique-pid.ts> crush packets identified by PID by packets of <unique-pid.ts>
    NOTE: -rep-pid -dup-pid -ins-pid -crush-pid commands are exclusive
    -toggle-tsc <pid> inverse the TSC bits of packets of selected PID

    special AIT
    -set-ait-id    <app-id>
    -set-ait-url   <url-base> <url-entry-file-name>
    -set-ait-dsmcc <transport-prototcol-label>

    special HLS
    -hls <duration>    split the input TS file into chunks of duration specified in seconds
                       and create corresponing m3u manifest
