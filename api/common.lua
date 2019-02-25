--------------------------------------------------
--
-- common.lua
-- created on 17/02/2019
-- by Grogir
-- Licence: GNU GPL
--
--------------------------------------------------

package.cpath="luasocket/?.dll"
package.path="luasocket/?.lua"
http=require("socket.http")
https=require("ssl.https")

function getfile(path)
	local f=io.open(path)
	local all=f:read("*a")
	f:close()
	return all
end

function getpage(fullpath)
	local proto,path=fullpath:match("^(https?)://(.+)$")
	assert(proto and path)
	f=io.open(path)
	if not f then
		f=io.open(path,"w+")
		assert(f,"can't create "..path)
		local page=proto=="https" and https.request(fullpath) or http.request(fullpath)
		assert(page,"no such webpage "..fullpath)
		f:write(page)
		f:seek("set",0)
		print(path.." downloaded")
	end
	local all=f:read("*a")
	f:close()
	return all
end

function mkdir(path)
	local sep=package.config:sub(1,1)
	if sep=="\\" then
		path=path:match("://(.+)$"):gsub("/",sep)
		os.execute("if not exist "..path.." mkdir "..path)
	else
		os.execute("mkdir -p "..path:match("://(.+)$"))
	end
end
