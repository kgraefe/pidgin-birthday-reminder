# Pidgin Birthday Reminder Changes

## Version 1.13 (2020-03-28)
- Update Slovak language

## Version 1.12 (2017-12-23)
- Fix translations with plural forms (Github #7)

## Version 1.11 (2017-03-14)
- Improve plural forms in translations
- Add Polish language file

## Version 1.10 (2017-02-14)
- Add Lithuanian language file

## Version 1.9 (2016-02-01)
- Ship AppStream metainfo file (Github #3)
- Fix Skypeweb support for non-English languages (Github #5)
- Re-scan buddies after three months

## Version 1.8 (2015-12-30)
- Add support for SkypeWeb accounts
- Update plugin author and website
- Add Greek translation
- Fix: Remove protocol option when plugin is unloaded

## Version 1.7 (2010-09-20)
- Do not longer expect ICS file to be well-formed (Launchpad #614244)
- Added Brazilian Portuguese, Czech, Dutch, Slovak and Tamil language files

## Version 1.6 (2010-05-08)
- Added hint in preferences menu when sounds are muted
- Added Turkish, Hebrew, Portuguese and Galician language files

## Version 1.5 (2010-03-20)
- Renamed project to pidgin-birthday-reminder
- Added tabs in the preferences window
- iCalendar files are forced to have the .ics extension now
- Added Spanish translation (many thanks to Christoph Miebach)

## Version 1.4 (2010-03-03)
- Added iCalendar export
- Added French translation (many thanks to Pierre Etchemaïté)

## Version 1.3 (2009-12-14)
- Fixed IM to wrong contact when using notifications
- Fixed dealing with February 29 (Launchpad #2909655)

## Version 1.2.1 (2009-08-17)
- The plugin did not refresh the buddy list icons on midnight

## Version 1.2 (2009-07-05)
- Added new birthday icons (many thanks to Alexey L.)
- Added an option to remind just once a day
- Added ReadMe-Files

## Version 1.1 (2009-05-15)
- Added Russian translation (many thanks to Alexander Logvinov)

## Version 1.0 (2009-02-13)
- (First?) final release
- Fixed a crash with `gtk_widget_destroy()`
- Fixed wrong calculation of days before alert
- If you switch an account to `online` the plugin will just check those buddies'
  birthday
- Fixed a translation issue in the German version

## Version 0.6 (2009-02-10)
- Fixed a bug which prevents the plugin from scanning buddies where the birthday
  was removed by hand
- Contacts will be handled in a better way now
- Buddy List will be shown when birthday check was triggert by the tray menu
  item
- Added the ability to disable the scan per account
- Birthday List will now be destroyed when unloading the plugin
- Added the ability to write an IM out of the birthday list
- If there are more then one notification they will be summarized
- The plugin can be installed to the .purple directory (which makes it possible
  to deliver a pre-compiled version for linux)
- The age will not longer be shown in the Birthday List if the year of birth is
  lower then 1900
- Code cleaned up

## Version 0.5 (2009-02-03)
- Added preferences
- Added a birthday list
- Fixed a bug with duplicate buddies

## Version 0.4 (2009-02-02)
- Play sound on birthday
- Show the age of a buddy
- Show the days to the buddies birthday
- Don't show the year of birth if it's before 1900
- Don't accept birthdays in the future
- Added some buddylist icons

## Version 0.3 (2009-01-30)
- Fixed birthday check on midnight
- Fixed birthday scan to only scan buddies on protocols that do support
  birthdays 
- Check birthday on change

## Version 0.2 (2009-01-26)
- First public version

## Version 0.1 (2008-12-24)
- Birthday Reminder was born.
