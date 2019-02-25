--------------------------------------------------
--
-- AutoComplete Generator
-- created on 17/02/2019
-- by Grogir
-- Licence: GNU GPL
--
--------------------------------------------------

dofile("common.lua")
respath="https://raw.githubusercontent.com/Resike/BlizzardInterfaceResources/master/Resources"
mkdir(respath.."/Data")

function trim(s)
	return s and s:match"^%s*(.*%S)"
end
function purge(s)
	return s and s:gsub("?",""):gsub("<.->",""):gsub('"',"'"):gsub("&amp;","&"):gsub("&#160;"," "):gsub("&nbsp;"," "):gsub("&#10;"," "):gsub("&#039;","'"):gsub("&quot;","'"):gsub("&lt;","<"):gsub("&gt;",">")
end
function cut(str)
	act=1
	while str:len()>act+99 do
		nl=str:find("\n",act)
		if not nl or nl>act+100 then
			str=str:sub(1,act+99).."\n"..str:sub(act+100)
			act=act+101
		else
			act=nl+1
		end
	end
	return str
end

-- 1. Get all entries we want to include

db={}
all=getpage(respath.."/Data/GlobalTypes.lua.txt") -- globals
for name,type in all:gmatch('(%S+) = "(%S+)"') do
	if type~="string" and type~="number" and type~="boolean" then
		db[name]={func=type=="function",ov={}}
	end
end
for _,file in ipairs({"/APILua.lua","/APIC.lua","/WidgetAPI.lua"}) do -- "module.func" functions
	all=getpage(respath..file)
	for name in all:gmatch("[^%s:%(%)]+") do
		db[name]=db[name] or {func=true,ov={}}
		local module,func=name:match("^([^.]-)%.?([^.]+)$")
		db[module]=nil -- adding those is unnecessary
		db[func]=db[func] or {func=true,ov={}}
	end
end
all=getfile("Ace3.txt") -- ace3 functions
for name in all:gmatch("%S+") do
	db[name]={func=true,ov={}}
end
all=getpage(respath.."/Events.lua") -- events
for name in all:gmatch('[^%s"]+') do
	db[name]={func=false,ov={}}
end

-- 2. Fill with standard lua doc

all=getfile("lua.xml")
for name,content in all:gmatch('Word name="([^"]+)"(.-)<Key') do
	if db[name] and content:find('<Overload retVal="') then
		local module,func=name:match("^([^.]-%.?)([^.]+)$")
		db[func].doc=trim(content:gsub("Â",""):gsub("·","."):gsub('"void"','"'..module..'"'))
	end
end

-- 3. Fill with wowpedia doc

website="https://wow.gamepedia.com"
mkdir(website)

all=getpage(website.."/World_of_Warcraft_API")..getpage(website.."/Widget_API")
done={}

for tags,link,name,info in all:gmatch('<dd>([^<]-)<a href="(.-API_.-)".->(.-)</a>(.-)</dd>') do
	if not done[link] then
		done[link]=true
		info=trim(purge(info)) or ""
		local sign=trim(info:match("^%(([^)]*)%)"))
		local desc=trim((sign and info:sub(#sign+2) or info):match("[() -]*(.-)$"))
		if desc and desc:match("^[aA]dded in") then print(name,desc) end
		if not desc or #desc<12 or desc:find("not yet docu") then
			desc=nil
		end
		if sign then
			sign="("..sign..")"
		end
		
		if link:sub(1,5)=="/API_" then
			local page=getpage(website..link)
			desc=trim(purge(page:match('<meta name="description" content="(.-)"/>'))) or desc
			sign=trim(purge(page:match('<div class="mw%-parser%-output">.-<h2>'):match("<pre>(.-)</pre>"))) or sign
		end
		
		if desc or sign then
			local module,func=name:match("^([^.:]-[.:]?)([^.:]+)$")
			if db[func] then
				db[func].ov[module]={sign=sign,desc=desc}
			end
		end
	end
end
print(website.." done")

-- 4. Fill with wowprogramming doc

website="http://wowprogramming.com"
mkdir(website.."/docs/api")

all=getpage(website.."/docs/api.html")

for link,name,info in all:gmatch('<tr><td><a href="(.-)">(.-)</a></td><td>(.-)<') do
	local page=getpage(website..link)
	local desc=trim(purge(page:match("<div class=\"api%-desc\">\n\n<p>(.-)</div>")))
	local sign=trim(purge(page:match("<div class=\"signature\">\n\n<p>(.-)</p>\n</div>")))
	
	if desc or sign then
		local module,func=name:match("^([^.:]-[.:]?)([^.:]+)$")
		if db[func] then
			db[func].ov[module]=db[func].ov[module] or {}
			local ov=db[func].ov[module]
			ov.sign=ov.sign or sign
			ov.desc=ov.desc or desc
		end
	end
end
print(website.." done")

-- 5. Write the file

fmodel=io.open("completion.model.xml")
model=fmodel:read("*a")
fmodel:close()
modelbegin,modelend=model:find("<!-- KEYWORDS -->",1,true)

sorted={}
for k,v in pairs(db) do
	if not db[k:lower()] or k==k:lower() then
		table.insert(sorted,k)
	end
end
table.sort(sorted)
f=io.open("completion.xml","w")
f:write(model:sub(1,modelbegin-1))
for _,name in ipairs(sorted) do
	local t=db[name]
	if t.doc then
		f:write('\t\t<KeyWord name="'..name..'" '..t.doc..'\n')
	elseif next(t.ov) then
		f:write('\t\t<KeyWord name="'..name..'" func="yes">\n')
		for module,func in pairs(t.ov) do
			local desc=func.desc and cut('\n'..func.desc) or ''
			if not func.sign then
				f:write('\t\t\t<Overload retVal="" descr="'..desc..'"/>\n')
			else
				local ret=cut(func.sign:match('.-=') or '')
				ret=#ret>0 and #module>0 and ret..' '..module or ret..module
				
				for args in func.sign:gmatch('%((.-)%)') do
					if trim(args) then
						f:write('\t\t\t<Overload retVal="'..ret..'" descr="'..desc..'">\n')
						for param in args:gmatch('([^,]+)') do
							f:write('\t\t\t\t<Param name="'..trim(param)..'"/>\n')
						end
						f:write('\t\t\t</Overload>\n')
					else
						f:write('\t\t\t<Overload retVal="'..ret..'" descr="'..desc..'"/>\n')
					end
				end
			end
		end
		f:write('\t\t</KeyWord>\n')
	else
		f:write('\t\t<KeyWord name="'..name..'" func="'..(t.func and 'yes' or 'no')..'"/>\n')
	end
end
f:write(model:sub(modelend+1))
f:close()

print("done")




