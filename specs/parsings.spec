Length:
	1.	[property One]
	2.	[property Two]
	5.	[propery Five]
	6.	[property Six]
	0.	[single]

Input:
	Present.	[property input] [if !One]
	Not present.

Output:
	Present.	[property output] [if !One && !input]
	Not present.

Background:
	Present.	[property background] [if One && !input || One && !output || Two && !input || Two && !output || Six]
	Not present.


Comment:
	Present before redirection or background.	[if input || output || background]
	Not present.	
	Present after redirection. 			[if !input && !output] 


