_cman_complete() {
    local cur opts command
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD - 1]}"
    
    if [ "${#COMP_WORDS[@]}" -gt "1" ]; then
	command=${COMP_WORDS[1]}
    fi
    
    case "$command" in 
	add)
	    opts="--batch -b --type -t --no-auto"
	    ;;
	get)
	    opts="--field -f --type -t"
	    ;;
	change)
	    opts="--master --type -t --field -f --no-auto"
	    ;;
	ls)
	    opts="--type --help -t -h"
	    ;;
	delete)
	    opts="--type --help -t -h"
	    ;;
    esac

    case "$prev" in 
	cman)
	    opts="ls add get change delete help --help -h"
	    ;;
	"--type"|"-t")
	    opts="login api"
	    ;;
	"--field"|"-f")
	    opts="uname pass service key apiname accname"
	    ;;
	add|get|change)
	    opts="--help -h"
	    ;;
    esac

    COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
    return 0;
}

complete -F _cman_complete cman

