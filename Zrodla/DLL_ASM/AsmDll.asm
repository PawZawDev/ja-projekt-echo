.DATA
.CODE
DllEntry PROC hInstDLL:DWORD, reason:DWORD, reserved1:DWORD
mov rax, 1
ret
DllEntry ENDP

Delay PROC Source: QWORD, dataSize: QWORD, destination: QWORD, myBegin: QWORD, myEnd: QWORD, delayStep: QWORD,iterationLength: QWORD
;Source - table address (not your begin)
;dataSize - number of bits (or bytes?) in source table
;destintion - addres of output table
;myBegin - starting point for this thread
;myBegin - ending point for this thread
;delayStep - range of one delay loop (adding one echo)


;initial for procedure - required
sub  rsp, 8
push rbx
push rbp

;move arguments from registers to variables
mov	Source, rcx
mov	dataSize, rdx
mov	destination, r8
mov	myBegin, r9
mov r10, myEnd
mov r12, delayStep
mov r14, delayStep
sub r14, iterationLength


mov r13,rcx ;move input addres 
add r13,rdx ;add input size (to make end of input)
mov iterationLength, r13

mov r15,r8 ;bufor do zapisu do przodu
add r15,r12

add r10,rcx ; zeby myend to byl koniec dla rcx
mov myEnd, r10

add rcx,r9
add r8,r9
add r15,r9

MainLoop:
	vmovdqu ymm0,ymmword ptr [rcx] ;wczytaj dane z inputa
	vmovdqu ymm1,ymmword ptr [r8] ;wczytaj dane z outputa

	VPSRAW YMM1, YMM1, 1 ;przesuniecie bitowe arytmetyczne w prawo o 1 bit (sciszenie)
	
	vpaddw ymm2,ymm1,ymm0 ;ymm2 - wynik to outputu
	vmovdqu ymmword ptr[r8],ymm2 ; zapis do outputu
	vmovdqu ymmword ptr[r15],ymm2 ; zapis do outputu do przodu
	
	add rcx,32 ;przesuniecie o 16 wartosci (rozmiar ymm)
	add r8, 32 
	add r15,32

mov	Source, rcx ;debug 
mov	destination, r8
mov	Source, rcx

	cmp rcx,r10 ; sprawdz czy input nie jest poza swoja granica
	jge addingNextDelay
	jmp MainLoop

addingNextDelay:
	add rcx, r14 ;przesuwa wskazniki do nastepnej iteracji o delaystep - length
	add r8, r14
	add r15, r14
	add r10,r12 ; tylko delaystep

mov	Source, rcx
mov	destination, r8
mov myEnd, r10
	
	cmp rcx,r13	;sprawdz czy input nie jest poza tablica
	jge finishProcedure
	jmp MainLoop

finishProcedure:
	pop rbp
	pop rbx
	
	add rsp,28	
	ret

Delay ENDP


End