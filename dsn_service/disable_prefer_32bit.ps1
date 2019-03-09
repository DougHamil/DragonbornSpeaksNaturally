# add Prefer32Bit=false to a csproj file

$file = 'dsn_service.csproj'

$doc = New-Object System.Xml.XmlDocument
$doc.Load($file)

$pgroups = $doc.DocumentElement.PropertyGroup
$pgroupCount = 0

for ($i=0; $i -le $pgroups.Count; $i++) {
	if ($pgroups[$i].PlatformTarget -eq "anycpu") {
		if ($null -eq $pgroups[$i].Prefer32Bit) {
			$child = $doc.CreateElement("Prefer32Bit", $doc.DocumentElement.xmlns)
			$child.InnerText = "false"
			$pgroups[$i].AppendChild($child) | out-null
		} else {
			$pgroups[$i].Prefer32Bit = "false"
		}

		$pgroupCount++
	}
}

$doc.Save($file)
"-- Set Prefer32Bit=false for $pgroupCount PropertyGroups"
