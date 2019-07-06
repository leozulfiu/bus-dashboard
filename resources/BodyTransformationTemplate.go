{
	"connections": [{{range $index, $element := .connections}}
		{{if $index}},{{end}} {
			"from": "{{$element.from}}",
			"departure": "{{$element.departure}}",
			"departure_delay": {{if $element.dep_delay}}"{{$element.dep_delay}}"{{else}}"+0"{{end}},
			"to": "{{$element.to}}"
		}{{end}}
	]
}
