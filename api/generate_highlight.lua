--------------------------------------------------
--
-- Syntax Highlighting Generator
-- created on 17/02/2019
-- by Grogir
-- Licence: GNU GPL
--
--------------------------------------------------

dofile("common.lua")
respath="https://raw.githubusercontent.com/Resike/BlizzardInterfaceResources/master/Resources"
datapath="https://raw.githubusercontent.com/Resike/BlizzardInterfaceResources/b6abf3b3b320a2a2f1054f20ddd17df6b1a0404c/Resources/Data"
mkdir(respath)
mkdir(datapath)

function exists(name)
	return luaglobals and luaglobals[name] or api and api[name] or ui and ui[name]
end
function getmap(all)
	local map,name={}
	for name in all:gmatch("[^%s:%(%)]+") do
		if not exists(name) then
			map[name:match("[^%.]+$")]=true
		end
	end
	return map
end
function getsortednames(map)
	local sorted,name,_={}
	for name,_ in pairs(map) do
		table.insert(sorted,name)
	end
	local txt=""
	table.sort(sorted)
	for _,name in ipairs(sorted) do
		txt=txt.." "..name
	end
	return txt:sub(2)
end


out=getfile("highlight.model.xml")


-- lua globals
luaglobals=getmap(getpage(respath.."/APILua.lua"))
out=out:gsub("<!%-%- LUAGLOBALS %-%->",getsortednames(luaglobals))

-- api
api=getmap(getpage(respath.."/API.lua").."\n"..getpage(respath.."/APIC.lua"))
out=out:gsub("<!%-%- API %-%->",getsortednames(api))

-- ui + ace3
ui=getmap(getpage(respath.."/WidgetAPI.lua").."\n"..getfile("Ace3.txt"))
out=out:gsub("<!%-%- UI %+ ACE3 %-%->",getsortednames(ui))

-- global tables
all=getpage(datapath.."/GlobalTypes.lua")
tables={}
for name,type in all:gmatch('(%S+) = "(%S+)"') do
	if type~="string" and type~="function" and type~="number" and type~="boolean" and not exists(name) then
		tables[name]=true
	end
end
out=out:gsub("<!%-%- TABLES %-%->",getsortednames(tables))


f=io.open("highlight.xml","w")
f:write(out)
f:close()

print("done")




