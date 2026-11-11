#Run this in a folder with several .csv benchmark output files
#and it will combine them into a single csv to make it easier to compare.
#This is useful when benchmarking different machines and putting all of their
#output csv files in a single folder.
$table = New-Object system.Data.DataTable “BenchAllTable”

$col0 = New-Object system.Data.DataColumn Machine, ([string])
$table.columns.add($col0)

function ProcessAllCsv()
{
    $b = $True;
    Get-ChildItem "./" -Filter *.csv | 
    Foreach-Object {
        $filename = $_.FullName

        if ($b -eq $True)
        {
            $b = $False

            $firstcsv = Import-Csv $filename

            foreach($csv in $firstcsv)
            {
                if ($csv.Device -eq "GPU" -or ($csv.Device -eq "CPU" -and $csv.Precision -eq "Double"))
                {
                    $colss1 = $csv.Filename + "_" + $csv.Precision + "_" + $csv.Device + "_SS1"
                    $colss2 = $csv.Filename + "_" + $csv.Precision + "_" + $csv.Device + "_SS2"
                    $colss4 = $csv.Filename + "_" + $csv.Precision + "_" + $csv.Device + "_SS4"

                    #$colss1
                    #$colss2
                    #$colss4

                    $col1 = New-Object system.Data.DataColumn $colss1, ([string])
                    $col2 = New-Object system.Data.DataColumn $colss2, ([string])
                    $col4 = New-Object system.Data.DataColumn $colss4, ([string])

                    $table.columns.add($col1)
                    $table.columns.add($col2)
                    $table.columns.add($col4)
                }
            }
        }

        $rowindex = 0
        $row = $table.NewRow()
        $mycsv = Import-Csv $filename
        $row[$rowindex++] = [io.path]::GetFileNameWithoutExtension($filename)

        foreach($csv in $mycsv)
        {
            if ($csv.Device -eq "GPU" -or ($csv.Device -eq "CPU" -and $csv.Precision -eq "Double"))
            {
                $row[$rowindex++] = '{0:N0}' -f [convert]::ToInt64($csv.SS1, 10)
                $row[$rowindex++] = '{0:N0}' -f [convert]::ToInt64($csv.SS2, 10)
                $row[$rowindex++] = '{0:N0}' -f [convert]::ToInt64($csv.SS4, 10)
            }
        }
        
        $table.Rows.Add($row)
    }

}

Remove-Item -Path ".\benchout_combine.csv" -Force
ProcessAllCsv
$table | Export-Csv -Path ".\benchout_combine.csv" -Force -NoTypeInformation