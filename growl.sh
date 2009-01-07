#install_name_tool -change @executable_path/../Frameworks/Growl.framework/Versions/A/Growl ~/.irssi/Growl.framework/Versions/A/Growl Growl.framework/Versions/Current/Growl
install_name_tool -id @loader_path/../Growl.framework/Versions/A/Growl Growl.framework/Versions/Current/Growl
rm -rf ~/.irssi/Growl.framework
cp -R Growl.framework ~/.irssi
