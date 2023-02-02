# input
	exit:
		true. [single] 
	EOF as input:
		true. [single]
	$? as input:
		true. [single]
	errored while inputting:
		true. [single]
	child process in background:
		true. [property childrunning]
		false.
	child process exited:
		true. [if childrunning] [property childexited]
		false.[if childrunning] [if !childexited]
	child process stopped:
		true. [if childrunning] [if !childexited] [property childstopped]
		false.[if childrunning] [if !childexited] [if !childstopped]
	child process signaled:
		true. [if childrunning] [if !childexited] [if !childstopped] [property childsignaled]
		false.[if childrunning] [if !childexited] [if !childstopped] [if !childsignaled]
	getline interrupted by signal:
		true. [if childrunning] [if !childexited] [if !childstopped] [if !childsignaled]
		false. [if childrunning] [if !childexited] [if !childstopped] [if !childsignaled]
	errno is set before running:
		true. [single] 
	user is root:
		true. 
		false.

