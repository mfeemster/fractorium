$exe = ""
if ([System.Environment]::OSVersion.Platform.ToString().ToLower() -like "*win*")
{
    # $exe = "./EmberRender.exe"
    $exe = "D:\Dev\fractorium\Bin\x64\Release\EmberRender.exe"
}
else
{
    $exe = "emberrender"
}

$benchprefix = "./bench/"
$devices = "0"#Set this to whatever device index your main GPU resides at. If you are unsure, just run emberrender --openclinfo to find out.
$cpuquality = 150
$gpuquality = 2000
$verbose = "--verbose"
$name_enable = "--name_enable"
$dump_args = ""#"--dumpargs"
$totalOutput = ""
$ssArray = @("1","2","4")
$ssSuffixArray = @("_ss1","_ss2","_ss4")

$Script:output = ""
[Collections.Generic.List[String]] $filteredLines = ""

$table = New-Object system.Data.DataTable "BenchTable"
$col1 = New-Object system.Data.DataColumn Filename, ([string])
$col2 = New-Object system.Data.DataColumn Precision, ([string])
$col3 = New-Object system.Data.DataColumn Device,([string])
$col4 = New-Object system.Data.DataColumn SS1, ([string])
$col5 = New-Object system.Data.DataColumn SS2, ([string])
$col6 = New-Object system.Data.DataColumn SS4, ([string])

#Add the Columns
$table.columns.add($col1)
$table.columns.add($col2)
$table.columns.add($col3)
$table.columns.add($col4)
$table.columns.add($col5)
$table.columns.add($col6)

function TestFileSupersamples([string]$filename, [string]$precision, [string]$suffix, [string]$quality, [string]$misc)
{
    [int]$sscount = 0;
    $row = $table.NewRow()
    $row.Filename = [io.path]::GetFileNameWithoutExtension($filename)

    if ($precision -eq "--sp")
    {
        $row.Precision = "Single"
    }
    else
    {
        $row.Precision = "Double"
    }

    if ($misc -like "*opencl*")
    {
        $row.Device = "GPU"
    }
    else
    {
        $row.Device = "CPU"
    }

    foreach ($ssval in $Script:ssArray)
    {
        $fullSuffix = $suffix + $Script:ssSuffixArray[$sscount]
        $renderargs = ("--in=$filename",  "$verbose", "$name_enable", "$dump_args", "--suffix=$fullSuffix", "--supersample=$ssval", "$precision", "--quality=$quality") + $misc.Split(" ")
        $Script:output = (&$exe $renderargs) | Out-String
        $val = $Script:output.split([environment]::NewLine) | where {$_ -like "*Iters/sec*"}
        $val = (($val.split(' ')[1] -replace '[.,]',''))
        $row[3 + $sscount] = $val
        $Script:filteredLines += $val
        $Script:totalOutput += [environment]::NewLine + "====================" + [environment]::NewLine + $Script:output
        $sscount++
    }

    $table.Rows.Add($row)
}

function BenchAllForFile([string]$filename)
{
    #if you want to test extreme speed on your GPU, add this option, --sbpctth=1.0, to increase the amount of each sub batch that is done on each opencl thread per kernel launch.
    #set the value from somewhere between 0.025 (the default) and 1.0 (the max). Values above 0.3 don't make much of a difference.
    #this usually results in a roughly 1% speed improvement.
    #however, it can cause the render to fail, especially on the golubaja_rippingfrominside_complexcode and zy0rg_six_bigcomplexcode flames when using double precision.
    $misc = "--opencl --device=" + $devices
    #Comment these first two out if you only want to test on the GPU.
    #TestFileSupersamples $filename "--sp" "_f32_cpu" $script:cpuquality ""
    #TestFileSupersamples $filename "" "_f64_cpu" $script:cpuquality ""
    TestFileSupersamples $filename "--sp" "_f32_opencl" $script:gpuquality $misc
    TestFileSupersamples $filename "" "_f64_opencl" $script:gpuquality $misc
}

cd ..

$fileOne = $benchprefix + "mfeemster_basicmemory.flame"
BenchAllForFile $fileOne

$fileOne = $benchprefix + "tatasz_springcrown_manysimplexforms.flame"
BenchAllForFile $fileOne

$fileOne = $benchprefix + "tyrantwave_flippeddisc_normal.flame"
BenchAllForFile $fileOne

$fileOne = $benchprefix + "golubaja_rippingfrominside_complexcode.flame"
BenchAllForFile $fileOne

$fileOne = $benchprefix + "zy0rg_six_bigcomplexcode.flame"
BenchAllForFile $fileOne

$Script:totalOutput | Out-File -FilePath benchout.txt
$table | Export-Csv -Path ".\benchout.csv" -Force -NoTypeInformation
