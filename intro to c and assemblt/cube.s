.global cube 
.text 
cube:
	enter $0, $0
	mov %rdi, %rax
	mov %rdi, %r8
	imul %r8
	imul %r8
	leave
	ret 
