# Copyright (c) 2021 Olle Lögdahl
# 
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT

# Compiles all *.b files and runs them with the corresponding .t
# file as in data and expectation.
# if the b file ends with _error it should not be able to compile.

# usage: check.sh out/bfc

BFC=$1
TMPDIR=$(mktemp -d)

[ -x $BFC ] || { echo "${BFC}: cannot execute"; exit 1; }

TESTS_RAN=0
SCENARIOS_RAN=0

for TFILE in tests/*.b ; do
    TESTS_RAN=$((TESTS_RAN+1))

    TNAME=$(basename $TFILE .b)
    DFILE="${TFILE%.*}.dat"

    echo -en "$TNAME:\t"

    # compile file
    $BFC -o "$TMPDIR/$TNAME.o" $TFILE 2> "$TMPDIR/log"
    RC=$?
    LINES=$(wc -l <"$TMPDIR/log")
    
    if [ $RC -ne 0 ] || [ $LINES -gt 0 ]; then
        echo fail
        cat $TMPDIR/log
        continue
    fi

    # link file to executable.
    ld -o "$TMPDIR/$TNAME" "$TMPDIR/$TNAME.o"
    
    if [ -f $DFILE ]; then
        IN_STR=""
        EXP_STR=""

        IN="$TMPDIR/$TNAME"_in
        touch $IN

        OUT="$TMPDIR/$TNAME"_out
        EXP="$TMPDIR/$TNAME"_exp

        SCENARIO_NO=0

        # Read all rules from the data file and evaluate them.
        while IFS="" read -r LINE || [ -n "$LINE" ]; do
            SCENARIOS_RAN=$((SCENARIOS_RAN+1))

            KEY=${LINE::1}
            case $KEY in
            '+')
                # Set expected result to the rule
                printf ${LINE:1} > $EXP
                ;;
            '-')
                # Set indata to the rule
                printf ${LINE:1} > $IN
                ;;
            '%')
                # Start evaluating the rule
                SCENARIO_NO=$((SCENARIO_NO+1))
                "$TMPDIR/$TNAME" < $IN > $OUT

                cmp -s $OUT $EXP || {
                    echo fail;
                    echo "unexpected output (scenario $SCENARIO_NO):";
                    echo -e "\texpected:\t" $(od -x $EXP)
                    echo -e "\tactual:  \t" $(od -x $OUT)
                    continue;
                }
                ;;
            esac
        done < $DFILE
    fi

    echo ok
done

echo "ran" $TESTS_RAN "tests with" $SCENARIOS_RAN "scenarios."
