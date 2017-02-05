{$A+,B-,D+,E-,F-,G+,I-,L+,N+,O-,P-,Q+,R+,S+,T-,V+,X+}
{$M 16384,0,655360}

program preprocessor;
uses crt, dos;

var outfile,infile:text;

function fileexists(filename: string): boolean;
var arq:file;
begin
  assign(arq, filename);
  reset(arq);
  close(arq);
  fileexists := (ioresult = 0) and (filename <> '');
end;

procedure include(str:string;var destiny:text);
var infile:text;
    line,line2,line3:string;
    x,x1:integer;
    ch:char;
    shift:boolean;
    startline:boolean;
begin
  assign(infile,str);
  reset(infile);
  while(not(eof(infile)))do begin
    readln(infile,line);
    shift:=true;
    startline:=true;
    x1:=1;
    for x:=1 to length(line) do begin
      ch:=line[x];
      if ch=#9 then ch:=' ';
      if (ch<>' ')or(line2[x1-1]<>' ')or(not(shift)) then begin
        if (not((ch=' ') and (startline))) then begin
          startline:=false;
          if (ch='"')or(ch='''')or(ch='<')or(ch='>') then shift:=not shift;
          if (ch<'a')or(ch>'z') then begin
            line2[x1]:=ch;
            inc(x1);
          end else begin
            if shift then ch:=upcase(ch);
            line2[x1]:=ch;
            inc(x1);
          end;
        end;
      end;
    end;
    line2[0]:=chr(x1-1);
    if (pos(';',line2)<>0)and(shift) then line2[0]:=chr(pos(';',line2)-1);
    while (line2[ord(line2[0])]=' ') do line2[0]:=chr(ord(line2[0])-1);
    x:=pos('INCLUDE ',line2);
    if(x=1)then begin
      line2:=copy(line2,9,200);
      if(fileexists(line2)) then include(line2,outfile) else begin
        for x:=1 to envcount do begin
          if pos('INCLUDE=',envstr(x))<>0 then begin
            line3:=line2;
            line2:=copy(envstr(x),pos('INCLUDE=',envstr(x))+8,200)+line2;
            if(fileexists(line2)) then begin
              include(line2,outfile);
              break;
             end else line2:=line3;
          end;
        end;
      end;
      line2:='';
    end;
    if line2<>'' then writeln(destiny,line2);
  end;
  close(infile);
end;

label 1;

var out,out2:string;
    line:string;
    counter,x:integer;
    token:array[1..9]of string;
    lable:string[6];
    x2:integer;
    debugon:boolean;

begin
  writeln('ASM Pre-processor 0.3 (c) Vecna 1999');
  if(paramcount<>2)then begin
    writeln('PREPROC.EXE <infile> <outfile>');
    halt(1);
  end;
  lable:='000000';
  out:=paramstr(2);
  assign(outfile,'TEMP.$$$');
  rewrite(outfile);
  include(paramstr(1),outfile);
  close(outfile);
  assign(infile,'TEMP.$$$');
  assign(outfile,'TEMP2.$$$');
  reset(infile);
  rewrite(outfile);
  debugon:=false;
  while(not(eof(infile)))do begin
    readln(infile,line);
    if(pos('DEBUGSTR ',line)<>0)then begin
      if debugon then begin
        writeln(outfile,'');
        counter:=0;
        for x:=1 to 9 do token[x]:='';
        line:=copy(line,pos(' ',line)+1,200);
        line:=line+',';
        while(pos(',',line)<>0)do begin
          inc(counter);
          if pos(',',line)<>0 then x:=pos(',',line)-1 else x:=200;
          out2:=copy(line,1,x);
          token[counter]:=out2;
          line:=copy(line,pos(',',line)+1,200);
        end;
        writeln(outfile,'PUSHFD');
        writeln(outfile,'PUSHAD');
        x2:=1;
        for x:=9 downto 2 do if token[x]<>'' then begin
          writeln(outfile,'PUSH '+token[x]);
          inc(x2);
        end;
        counter:=6;
       1:
        inc(lable[counter]);
        if lable[counter]>'9' then begin
          lable[counter]:='0';
          counter:=counter-1;
          goto 1
        end;
        writeln(outfile,'CALL @@'+lable);
        writeln(outfile,'DB "'+token[1]+'",13,10,0');
        writeln(outfile,'@@'+lable+':');
        writeln(outfile,'CALL DEBUG_OUTPUT');
        str(x2*4,out2);
        writeln(outfile,'ADD ESP, '+out2);
        writeln(outfile,'POPAD');
        writeln(outfile,'POPFD');
        writeln(outfile,'');
      end else line:='';
    end else if pos('DEBUGON',line)<>0 then begin
      debugon:=TRUE;
      writeln(outfile,'DEBUG EQU TRUE');
      line:='';
    end else if pos('DEBUGOFF',line)<>0 then begin
      debugon:=FALSE;
      writeln(outfile,'DEBUG EQU FALSE');
      line:='';
    end else if (pos('BREAK',line)<>0)and(debugon) then begin
      writeln(outfile,'DB 0CCH');
      line:='';
    end else if (pos('FREE4GARBLE',line)<>0) then begin
      writeln(outfile,'DB 0EBH,4');
      writeln(outfile,'DB 56h,56h');
      line:='DW '+copy(line,pos(' ',line)+1,200);
      writeln(outfile,line);
      line:='';
    end;
    if line<>'' then writeln(outfile,line);
  end;
  close(infile);
  close(outfile);
  erase(infile);
  assign(outfile,out);
  rewrite(outfile);
  include('TEMP2.$$$',outfile);
  close(outfile);
  assign(outfile,'TEMP2.$$$');
  erase(outfile);
end.
