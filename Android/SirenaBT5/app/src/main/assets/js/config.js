'use strict'
var jsi = {};

jsi.getConfig =
function ()
{
	let build = 1778;

	let s = "{\"pincode\":\"0000\",";
	s += "\"build\":" + build + ",";
	s += "\"version\":5.0,";
	s += "\"melodyMax\":40,";
	s += "\"eventMax\":30,";
	s += "\"volume\":3,";
	s += "\"volumeBeep\":5,";
	s += "\"sleep\":true,";

	if (build >= 1777)
	{
		s += "\"serial\":\"SBT5-00000000-00000000-00000000\",";
	}

	s += "\"channels\":[";

	if (build >= 1331)
	{
		s += "{\"index\":0,";
		s += "\"pressMax\":500,";
		s += "\"idleMax\":1000,";
		s += "\"pulsesMax\":20,";
		s += "\"deviation\":20,";
		s += "\"button\":true,";
		s += "\"measure\":false,";
		s += "\"playOnce\":false,";
		s += "\"playToEnd\":false,";
		s += "\"noWidth\":true";
        if (build >= 1513)
        {
            s += ",\"noBounceHigh\":true";
        }
        if (build >= 1778)
        {
            s += ",\"standard\":true";
        }
		s += "},{\"index\":1,";
		s += "\"pressMax\":500,";
		s += "\"idleMax\":1000,";
		s += "\"pulsesMax\":20,";
		s += "\"deviation\":20,";
		s += "\"button\":true,";
		s += "\"measure\":false,";
		s += "\"playOnce\":false,";
		s += "\"playToEnd\":false,";
		s += "\"noWidth\":true";
        if (build >= 1513)
        {
            s += ",\"noBounceHigh\":true";
        }
        if (build >= 1778)
        {
            s += ",\"standard\":false";
        }
		s += "},{\"index\":2,";
		s += "\"pressMax\":500,";
		s += "\"idleMax\":1000,";
		s += "\"pulsesMax\":20,";
		s += "\"deviation\":20,";
		s += "\"button\":true,";
		s += "\"measure\":false,";
		s += "\"playOnce\":false,";
		s += "\"playToEnd\":true,";
		s += "\"noWidth\":true";
        if (build >= 1513)
        {
            s += ",\"noBounceHigh\":true";
        }
        if (build >= 1778)
        {
            s += ",\"standard\":false";
        }
		s += "}";
	}
	else
	{
		s += "{\"index\":0,";
		s += "\"pressMax\":500,";
		s += "\"idleMax\":1000,";
		s += "\"pulsesMax\":10,";
		s += "\"deviation\":20,";
		s += "\"button\":true,";
		s += "\"measure\":false}";
	}

	s += "],\"melodys\":[";

	s += "{\"index\":3,";
	s += "\"name\":\"Тест 3\"},";
	s += "{\"index\":0,";
	s += "\"name\":\"Тест 0\"},";
	s += "{\"index\":1,";
	s += "\"name\":\"Тест 1\"},";
	s += "{\"index\":11,";
	s += "\"name\":\"Тест 11\"},";
	s += "{\"index\":12,";
	s += "\"name\":\"Тест 12\"},";
	s += "{\"index\":13,";
	s += "\"name\":\"Тест 13\"},";
	s += "{\"index\":14,";
	s += "\"name\":\"Тест 14\"},";
	s += "{\"index\":15,";
	s += "\"name\":\"Тест 15\"},";
	s += "{\"index\":16,";
	s += "\"name\":\"Тест 16\"},";
	s += "{\"index\":17,";
	s += "\"name\":\"Тест 17\"},";
	s += "{\"index\":18,";
	s += "\"name\":\"Тест 18\"},";
	s += "{\"index\":19,";
	s += "\"name\":\"Тест 19\"}";

	s += "],\"events\":[";


	s += "{\"index\":0,";
	s += "\"chIndex\":1,";
	s += "\"melodyIndex\":0,";
	s += "\"poweroff\":false,";
	if (build >= 1513)
	{
		s += "\"volume\":9,";
	}
	if (build >= 1590)
	{
		s += "\"noPlayAfter\":-1,";
	}
	s += "\"type\":4,";
	s += "\"pulse\":[";
		s += "{\"width\":100,";
		s += "\"count\":3},";
		s += "{\"width\":101,";
		s += "\"count\":4},";
		s += "{\"width\":102,";
		s += "\"count\":5}";
	s += "]},";


	s += "{\"index\":1,";
	s += "\"chIndex\":3,";
	s += "\"melodyIndex\":-1,";
	s += "\"poweroff\":true,";
	if (build >= 1513)
	{
		s += "\"volume\":4,";
	}
	if (build >= 1590)
	{
		s += "\"noPlayAfter\":-1,";
	}
	s += "\"type\":1,";
	s += "\"pulse\":[";
	s += "]}";


	s += "]}";


	return s.split('').join('');
};

jsi.speakStart = function () {console.log('speakStart');ui.cbSpeak(true)};
jsi.speakStop = function () {console.log('speakStop');setTimeout(function () {ui.cbSpeak(false);}, 1000);};
jsi.play = function (index) {console.log('play');ui.cbPlay(true, index); setTimeout(function () {ui.cbPlay(false, index);}, 1000);};
jsi.stop = function () {console.log('stop');ui.cbPlay(false, -1);};
jsi.deleteMelody = function (index) {console.log('deleteMelody');ui.cbDeleteMelody(index);};
jsi.deleteEvent = function (index) {console.log('deleteEvent');ui.cbDeleteEvent(index);};
jsi.setPressMax = function (index, value) {console.log('setPressMax', index, value);setTimeout(function () {ui.cbSetPressMax(index);}, 500);};
jsi.setIdleMax = function (index, value) {console.log('setIdleMax', index, value);setTimeout(function () {ui.cbSetIdleMax(index);}, 500);};
jsi.setPulsesMax = function (index, value) {console.log('setPulsesMax', index, value);setTimeout(function () {ui.cbSetPulsesMax(index);}, 500);};
jsi.setDeviation = function (index, value) {console.log('setDeviation', index, value);setTimeout(function () {ui.cbSetDeviation(index);}, 500);};
jsi.setButton = function (index, value) {console.log('setButton', index, value);setTimeout(function () {ui.cbSetButton(index);}, 500);};
jsi.setMeasure = function (index, value) {console.log('setMeasure', index, value);setTimeout(function () {ui.cbSetMeasure(index);}, 500);};
jsi.setPlayOnce = function (index, value) {console.log('setPlayOnce', index, value);setTimeout(function () {ui.cbSetPlayOnce(index);}, 500);};
jsi.setPlayToEnd = function (index, value) {console.log('setPlayToEnd', index, value);setTimeout(function () {ui.cbSetPlayToEnd(index);}, 500);};
jsi.setNoWidth = function (index, value) {console.log('setNoWidth', index, value);setTimeout(function () {ui.cbSetNoWidth(index);}, 500);};
jsi.setNoBounceHigh = function (index, value) {console.log('setNoBounceHigh', index, value);setTimeout(function () {ui.cbSetNoBounceHigh(index);}, 500);};
jsi.setStandard = function (index, value) {console.log('setStandard', index, value);setTimeout(function () {ui.cbSetStandard(index);}, 500);};
jsi.setVolume = function (value) {console.log('setVolume', value);setTimeout(function () {ui.cbSetVolume();}, 500);};
jsi.setVolumeBeep = function (value) {console.log('setVolumeBeep', value);setTimeout(function () {ui.cbSetVolumeBeep();}, 500);};
jsi.setSleep = function (value) {console.log('setSleep', value);setTimeout(function () {ui.cbSetSleep();}, 500);};
jsi.setPINCode = function (pin) {console.log('setPINCode');ui.cbSetPINCode(true)};
jsi.saveFS = function () {console.log('saveFS');ui.cbSaveFS();};
jsi.saveConfig = function () {console.log('saveConfig');ui.cbSaveConfig();};
jsi.setEvent = function (event) {console.log('setEvent');let json = JSON.parse(event); ui.cbChangeEvent(json.index);};
jsi.getLang = function () { return 'ru'; }

function getRandomInt(min, max)
{
	return Math.floor(Math.random() * (max + 1 - min)) + min;
}

jsi.getLogEvent =
	function ()
	{
		let evt =
			{
				chIndex : getRandomInt(1, 3),
				type : getRandomInt(1, 6)
			};
		if (evt.type == 4)
		{
			evt.pulse = [];
			for (let i = 0; i < getRandomInt(1, 3); i++)
			{
				evt.pulse.push({ 'width' : getRandomInt(100, 500), 'count' : getRandomInt(1, 10) });
			}
		}
		else if (evt.type == 5)
		{
			evt.measure = {};
			evt.measure.width = getRandomInt(100, 500);
			evt.measure.idle = getRandomInt(evt.measure.width, evt.measure.width + 500);
		}
		return JSON.stringify(evt);
	};

jsi.setButtons = function (data) {};
jsi.getButtons =
	function ()
	{
		let b = {};
		b.emoji = [0,1,2,3,4,5,6,7];
		b.mld = [-1,-1,-1,-1,-1,-1,-1,-1];
		return JSON.stringify(b);
	};

jsi.getDownloadFiles =
	function ()
	{
		let f =
			{
				"catalog" : "Download",
				"files" : ["1.WAV", "2.wav"]
			};
		return JSON.stringify(f);
	};

jsi.getEventName =
	function ()
	{
		return '{"1":"Test Event Name"}';
	};

jsi.setEventName =
	function (j)
	{
		console.log('setEventName', j);
	};

jsi.setUpdate =
	function (fw, hash)
	{
		console.log('setUpdate', hash, fw);
		return true;
	};

jsi.checkSerial =
	function (hash)
	{
		console.log('checkSerial', hash);
	};

setTimeout(function () {ui.cbButtons();ui.cbEventName();ui.cbEvent();ui.cbEvent();ui.cbEvent();ui.cbEvent();ui.cbEvent();ui.cbEvent();ui.cbEvent();ui.cbEvent();}, 100);