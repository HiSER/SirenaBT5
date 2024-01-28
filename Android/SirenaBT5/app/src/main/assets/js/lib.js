'use strict';
(function (w, d) { var ui = {};
	
	const emoji =
		[
			'1', '2', '3', '4', '5', '6', '7', '8',
			'&#x1F601;', '&#x1F602;', '&#x1F606;', '&#x1F609;', '&#x1F60A;', '&#x1F60B;', '&#x1F60D;', '&#x1F612;',
			'&#x1F616;', '&#x1F61C;', '&#x1F61D;', '&#x1F61E;', '&#x1F620;', '&#x1F621;', '&#x1F622;', '&#x1F624;',
			'&#x1F628;', '&#x1F629;', '&#x1F62B;', '&#x1F62D;', '&#x1F631;', '&#x1F525;', '&#x1F633;', '&#x1F635;',
			'&#x1F637;', '&#x1F64C;', '&#x1F64F;', '&#x270A;', '&#x270B;', '&#x270C;', '&#x2744;', '&#x2757;',
			'&#x2753;', '&#x2764;', '&#x2795;', '&#x2796;', '&#x2049;', '&#x1F692;', '&#x1F691;', '&#x1F695;',
			'&#x1F697;', '&#x1F699;', '&#x1F69A;', '&#x1F6A2;', '&#x1F6A5;', '&#x1F6A8;', '&#x1F6AB;', '&#x1F6B2;',
			'&#x1F6B6;', '&#x1F6BD;', '&#x1F693;', '&#x231A;', '&#x2600;', '&#x2601;', '&#x260E;', '&#x2615;',
			'&#x261D;', '&#x267F;', '&#x26A1;', '&#x2693;', '&#x1F37A;', '&#x26BD;', '&#x1F384;'
		]

	const SirenaPulse =
		{
			Null			: 0,
			Press			: 1,
			Release			: 2,
			Cyclic			: 3,
			Pulse			: 4,
			Measure			: 5,
			MeasureError	: 6
		};

	var Buttons =
		{
			emoji		: [ 0, 1, 2, 3, 4, 5, 6, 7 ],
			mld			: [ -1, -1, -1, -1, -1, -1, -1, -1 ],
			tmp			: { emoji : null, mld : null }
		}

	var Events = null;

	var vars =
		{
			version			: 0,
			build			: 0,
			melodyMax		: 0,
			eventMax		: 0,
			eventEdit		: -1,
			eventFromLog	: null,
			speakEnable		: true,
			melodys			: 0,
			pack			:
				{
					local		: false,
					current		: null,
					remote		: null,
					file		: null,
					audio		: null
				}
		};

	var eventName = {};

	function apiUpdateURL()
	{
		return 'https://artemlitvin.com/app/beephorn/' + vars.version.toFixed(1) + '/api/update/';
	}

	function apiMelodyURL()
	{
		return 'https://artemlitvin.com/app/beephorn/' + vars.version.toFixed(1) + '/api/melody/';
	}

	ui.isPage =
		function (name)
		{
			return (d.querySelector('div.page[name=' + name + ']').classList.contains('show'));
		};
	
	ui.onClickMenu =
		function (forceHide)
		{
			let buttonMenu = d.querySelector('#buttonMenu');
			if (buttonMenu.classList.contains('imgMenu'))
			{
				let menu = d.querySelector('.menu');
				if (forceHide === true || menu.classList.contains('show'))
				{
					buttonMenu.classList.remove('select');
					menu.classList.remove('show');
				}
				else
				{
					buttonMenu.classList.add('select');
					menu.classList.add('show');
				}
			}
			else if (forceHide === undefined)
			{
				if (buttonMenu.classList.contains('imgBack'))
				{
					let back = buttonMenu.getAttribute('back');
					if (back)
					{
						buttonMenu.removeAttribute('back');
						ui.onClickMenuItem(back);
					}
					else
					{
						ui.log('MENU Unknown back page');
					}
				}
			}
		};

	ui.onClickSubmenu =
		function (e)
		{
			let buttonSubmenu = d.querySelector('#buttonSubmenu');
			let topage = buttonSubmenu.getAttribute('topage');
			if (buttonSubmenu.classList.contains('imgPause'))
			{
				jsi.stop();
			}
			else if (ui.isPage('MainOption'))
			{
				ui.changeMainOption();
				ui.setButtons();
				ui.updateMain();
				ui.onClickMenuItem('Main');
			}
			else if (ui.isPage('MelodyAdd'))
			{
				ui.startLoadMelody();
				ui.onClickMenuItem('Melody');
			}
			else if (ui.isPage('Option'))
			{
				ui.onContextMenuOption.call(this, e);
			}
			else if (ui.isPage('PINCode'))
			{
				ui.setPINCode();
			}
			else if (ui.isPage('EventAdd'))
			{
				ui.eventApplyValue();
				ui.onClickMenuItem('Event');
			}
			else if (ui.isPage('Log'))
			{
				ui.onContextMenuLog.call(this, e);
			}
			else if (topage)
			{
				buttonSubmenu.removeAttribute('topage');
				ui.onClickMenuItem(topage);
			}
			else
			{
				ui.log('SUBMENU unknown action');
			}
		};

	ui.onClickMenuItem =
		function (item, e)
		{
			if (typeof(item) == 'string')
			{
				for (let page of d.querySelectorAll('.page.show'))
				{
					page.classList.remove('show');
				}
				for (let page of d.querySelectorAll('.menuItem.select'))
				{
					page.classList.remove('select');
				}
				let page = d.querySelector('.page[name=' + item + ']');
				if (page)
				{
					page.classList.add('show');

					let menuItem = d.querySelector('.menuItem[name=' + item + ']')
					if (menuItem) menuItem.classList.add('select');

					ui.onClickMenu(true);

					let headerText = d.querySelector('.headerText');
					let headerTextSub = d.querySelector('.headerTextSub');
					headerText.classList.removeRegExp('str.+');
					headerTextSub.innerHTML = '';
					let header = page.getAttribute('header');
					if (header)
					{
						headerText.classList.add('str' + header);
					}
					else
					{
						headerText.classList.add('str' + item);
					}

					let menu = page.getAttribute('menu');
					let buttonMenu = d.querySelector('#buttonMenu');
					buttonMenu.classList.removeRegExp('img.+');
					if (menu)
					{
						let buttonMenu = d.querySelector('#buttonMenu');
						buttonMenu.classList.removeRegExp('img.+');
						buttonMenu.classList.add('img' + menu);
						let back = page.getAttribute('back');
						if (back) buttonMenu.setAttribute('back', back);
					}
					else
					{
						buttonMenu.classList.add('imgMenu');
					}

					let buttonSubmenu = d.querySelector('#buttonSubmenu');
					buttonSubmenu.classList.removeRegExp('img.+');
					let submenu = page.getAttribute('submenu');
					if (submenu)
					{
						buttonSubmenu.classList.add('img' + submenu);
						let topage = page.getAttribute('topage');
						if (topage) buttonSubmenu.setAttribute('topage', topage);
					}

					let onshow = page.getAttribute('onshow');
					if (onshow)
					{
						eval(onshow);
					}
				}
			}
			else
			{
				e.preventDefault();
				e.stopPropagation();
				ui.onClickMenuItem(item.getAttribute('name'));
			}
		};

	ui.headerTextSubMelodyTotal =
		function ()
		{
			let headerTextSub = d.querySelector('.headerTextSub');
			let count = vars.melodys;
			headerTextSub.innerHTML = count + '/' + vars.melodyMax;
		};

	ui.headerTextSubEventTotal =
		function ()
		{
			let headerTextSub = d.querySelector('.headerTextSub');
			let count = (Events != null) ? Events.length : 0;
			headerTextSub.innerHTML = count + '/' + vars.eventMax;
		};

	ui.onPlay =
		function (index)
		{
			let mld = Buttons.mld[index];
			if (mld != -1) jsi.play(mld);
		};

    ui.cbPlay =
    	function (isPlay, index)
    	{
			if (ui.isPage('Main'))
			{
				let buttonSubmenu = d.querySelector('#buttonSubmenu');
				buttonSubmenu.classList.removeRegExp('img.+');
				if (isPlay)
				{
					buttonSubmenu.classList.add('imgPause');
				}
				else
				{
					buttonSubmenu.classList.add('imgOption');
				}
			}
			else if (ui.isPage('Melody'))
			{
				let buttonSubmenu = d.querySelector('#buttonSubmenu');
				buttonSubmenu.classList.removeRegExp('img.+');
				if (isPlay)
				{
					buttonSubmenu.classList.add('imgPause');
					d.querySelector('#melody' + index).classList.add('played');
				}
				else
				{
					buttonSubmenu.classList.add('imgAdd');
					if (index == -1)
					{
						for(let mld of d.querySelectorAll('[id^=melody]'))
						{
							mld.classList.remove('played');
						}
					}
					else
					{
						d.querySelector('#melody' + index).classList.remove('played');
					}
				}
			}
    	};

	ui.onSpeakStart =
    	function ()
    	{
    		if (vars.speakEnable)
    		{
    			vars.speakEnable = false;
				jsi.speakStart();
			}
    	};

	ui.onSpeakStop =
    	function ()
    	{
			jsi.speakStop();
    	};

    ui.cbSpeak =
    	function (isStart)
    	{
    		let buttonMic = d.querySelector('#buttonMic');
			if (isStart)
			{
				buttonMic.classList.add('disable');
			}
			else
			{
				vars.speakEnable = true;
				buttonMic.classList.remove('disable');
			}
    	};

	ui.log =
		function (text)
		{
			console.log(text);
		};

	ui.Message =
		function (message, noclose)
		{
			var msg = d.querySelector('div.message');
			let p = msg.querySelector('p');
			p.classList.removeRegExp('str.+');
			p.classList.add(message);
			msg.classList.add('show');
			msg._noclose = noclose;
			msg._handle = setTimeout(
				function ()
				{
					msg.classList.remove('show');
				}, 3000);
		};

	ui.MessageClose =
		function (e)
		{
			if (!(e && this._noclose === true))
			{
				clearTimeout(this._handle);
				this.classList.remove('show');
			}
		};

	ui.cbDeleteMelody =
		function (index)
		{
			if (index == -1)
			{
				for (let mld of d.querySelectorAll('[id^=melody]'))
				{
					mld.remove();
				}
			}
			else
			{
				let mld = d.querySelector('#melody' + index);
				mld.remove();
			}
			vars.melodys--;
			ui.headerTextSubMelodyTotal();
		};

	ui.cbChangeEvent =
		function (index)
		{

		};

	function removeEvent(index)
	{
		for (let i = 0; i < Events.length; i++)
		{
			if (index == Events[i].index)
			{
				d.querySelector('#event' + index).remove();
				Events.splice(i, 1);
				break;
			}
		}
	}

	ui.cbDeleteEvent =
		function (index)
		{
			if (index == -1)
			{
				for (let evt of d.querySelectorAll('[id^=event]'))
				{
					index = parseInt(evt.id.match(/^event(\d+)/)[1]);
					removeEvent(index);
				}
			}
			else
			{
				removeEvent(index);
			}
			ui.headerTextSubEventTotal();
		};

	ui.cbSetButton =
		function (index)
		{
			d.querySelector('#ch' + index + 'CHButton').disabled = false;
		};

	ui.cbSetMeasure =
		function (index)
		{
			d.querySelector('#ch' + index + 'CHMeasure').disabled = false;
		};

	ui.cbSetPressMax =
		function (index)
		{
			d.querySelector('#ch' + index + 'PressMax').disabled = false;
		};

	ui.cbSetIdleMax =
		function (index)
		{
			d.querySelector('#ch' + index + 'IdleMax').disabled = false;
		};

	ui.cbSetPulsesMax =
		function (index)
		{
			d.querySelector('#ch' + index + 'PulsesMax').disabled = false;
		};

	ui.cbSetDeviation =
		function (index)
		{
			d.querySelector('#ch' + index + 'Deviation').disabled = false;
		};

	ui.cbSetPlayOnce =
		function (index)
		{
			d.querySelector('#ch' + index + 'PlayOnce').disabled = false;
		};

	ui.cbSetPlayToEnd =
		function (index)
		{
			d.querySelector('#ch' + index + 'PlayToEnd').disabled = false;
		};

	ui.cbSetNoWidth =
		function (index)
		{
			d.querySelector('#ch' + index + 'NoWidth').disabled = false;
		};

	ui.cbSetNoBounceHigh =
		function (index)
		{
			d.querySelector('#ch' + index + 'NoBounceHigh').disabled = false;
		};

	ui.cbSetStandard =
		function (index)
		{
			d.querySelector('#ch' + index + 'Standard').disabled = false;
		};

	ui.cbSetVolume =
		function ()
		{
			d.querySelector('#cfgVolume').disabled = false;
		};

	ui.cbSetVolumeBeep =
		function ()
		{
			d.querySelector('#cfgVolumeBeep').disabled = false;
		};

	ui.cbSetSleep =
		function ()
		{
			d.querySelector('#cfgSleep').disabled = false;
		};

	ui.setConfig =
		function (e)
		{
			if (e && e.keyCode != 13) return;
			let m = this.id.match(/^(ch(\d+)|cfg)(.+)/);
			if (m)
			{
				this.disabled = true;
				let index = parseInt(m[2]);
				let name = m[3];
				if (this.getAttribute('type') == 'checkbox')
				{
					let value = this.checked;
					switch (name)
					{
						case 'CHButton':
							if (index == 0 || (vars.build >= 1778 && index == 1))
								d.querySelector('#ch' + index + 'CHMeasure').checked = false;
							jsi.setButton(index, value);
							break;
						case 'CHMeasure':
							d.querySelector('#ch' + index + 'CHButton').checked = false;
							jsi.setMeasure(index, value);
							break;
						case 'PlayOnce':
							jsi.setPlayOnce(index, value);
							break;
						case 'PlayToEnd':
							jsi.setPlayToEnd(index, value);
							break;
						case 'NoWidth':
							jsi.setNoWidth(index, value);
							break;
						case 'NoBounceHigh':
							jsi.setNoBounceHigh(index, value);
							break;
						case 'Standard':
							jsi.setStandard(index, value);
							break;
						case 'Sleep':
							jsi.setSleep(value);
							break;
					}
				}
				else
				{
					let min = parseInt(this.getAttribute('min'));
					let max = parseInt(this.getAttribute('max'));
					let value = parseInt(this.value);
					if (min > value) value = min;
					if (max < value || isNaN(value)) value = max;
					this.value = value;
					switch (name)
					{
						case 'PressMax':
							jsi.setPressMax(index, value);
							break;
						case 'IdleMax':
							jsi.setIdleMax(index, value);
							break;
						case 'PulsesMax':
							jsi.setPulsesMax(index, value);
							break;
						case 'Deviation':
							jsi.setDeviation(index, value);
							break;
						case 'Volume':
							jsi.setVolume(value);
							break;
						case 'VolumeBeep':
							jsi.setVolumeBeep(value);
							break;
					}
				}
			}
		};

	ui.onContextMenuClose =
		function (e)
		{
			if (e)
			{
				e.preventDefault();
				e.stopPropagation();
			}
			let context = d.querySelector('div.contextMenu');
			if (context.classList.contains('show'))
			{
				let item = context.getAttribute('item');
				if (item)
				{
					context.removeAttribute('item');
					d.querySelector(item).classList.remove('active');
				}
				context.classList.remove('show');
			}
		};

	ui.onContextMenu =
		function (e, activeItem, items)
		{
			e.stopPropagation();
			ui.onContextMenuClose();
			let context = d.querySelector('div.contextMenu');
			let activity = d.querySelector('div.activity');
			let left = this.offsetLeft + this.offsetWidth / 2;
			let top = this.offsetTop - ((activeItem) ? activity.scrollTop : 0)  + this.offsetHeight / 2;
			context.innerHTML = '';
			context.append(items);
			context.classList.add('show');
			if ((left + context.offsetWidth) > w.innerWidth)
			{
				left -= context.offsetWidth;
			}
			if ((top + context.offsetHeight) > w.innerHeight)
			{
				top -= context.offsetHeight;
			}
			context.style.left = left + 'px';
			context.style.top = top + 'px';
			if (activeItem)
			{
				context.setAttribute('item', activeItem);
				d.querySelector(activeItem).classList.add('active');
			}
		};

	ui.onContextMenuOption =
		function (e)
		{
			let items = d.createElement('div');
			let item;
			var evt = this;
			if (vars.build < 1778)
			{
				item = d.createElement('div');
				item.classList.add('strSaveFS');
				item.addEventListener('click',
					function (e)
					{
						ui.onContextMenuClose(e);
						jsi.saveFS();
					});
				items.append(item);
			}
			item = d.createElement('div');
			item.classList.add('strSaveConfig');
			item.addEventListener('click',
				function (e)
				{
					ui.onContextMenuClose(e);
					jsi.saveConfig();
				});
			items.append(item);
			item = d.createElement('div');
			item.classList.add('strPINCode');
			item.addEventListener('click',
				function (e)
				{
					ui.onContextMenuClose(e);
					ui.onClickMenuItem('PINCode');
				});
			items.append(item);
			ui.onContextMenu.call(this, e, null, items);
		};

	ui.cbSaveConfig =
		function ()
		{
			ui.Message('strSavedConfig');
		};

	ui.cbSaveFS =
		function ()
		{
			ui.Message('strSavedFS');
		};

	ui.cbSetPINCode =
		function (error)
		{
			if (error) ui.Message('strErrorPIN');
			d.querySelector('#PINCode').disabled = false;
		};

	ui.setPINCode =
		function ()
		{
			let PINCode = d.querySelector('#PINCode');
			let pin = PINCode.value;
			if (pin.match(/^\d{4}$/))
			{
				PINCode.disabled = true;
				jsi.setPINCode(pin);
				ui.onClickMenuItem('Option');
			}
			else
			{
				PINCode.classList.add('error');
			}
		};

	ui.getMelodyName =
		function (index)
		{
			let mld = d.querySelector('#melody' + index);
			let s;
			if (mld)
			{
				s = mld.querySelector('p').innerHTML;
			}
			return s;
		};

	ui.getNoPlayAfterName =
		function (index)
		{
			let evt = d.querySelector('#event' + index);
			let s = evt.querySelector('div.channel > p').innerHTML;
			s += ' - ';
			if (evt)
			{
				let name = evt.querySelector('p.name').innerHTML;
				s += (name) ? name : evt.querySelector('p.type').innerHTML;
			}
			return s.split('').join('');
		};

	ui.createListItem =
		function (text, value, isSelect, addClass)
		{
			let item = d.createElement('div');
			let p = d.createElement('p');
			item.setAttribute('value', value);
			if (isSelect === true) item.classList.add('select');
			p.innerHTML = text;
			if (addClass) p.classList.add(addClass);
			item.append(p);
			return item;
		};

	ui.createList =
		function (list, noRows)
		{
			let l = d.querySelector('div.list');
			l.innerHTML = '';
			if (noRows === true)
			{
				l.classList.add('rows');
			}
			else
			{
				l.classList.remove('rows');
			}
			l.append(list);
			l.classList.add('show');
		};

	ui.createListClose =
		function ()
		{
			let l = d.querySelector('div.list');
			l.classList.remove('show');
		};

	ui.onButtonEmoji =
		function ()
		{
			let indexButton = parseInt(this.parentNode.getAttribute('index'));
			let list = d.createElement('div');

			list.setAttribute('value', indexButton);

			let onchange =
				function (e)
				{
					e.stopPropagation();
					ui.createListClose();
					let index = parseInt(this.parentNode.getAttribute('value'))
					let em = parseInt(this.getAttribute('value'));
					Buttons.tmp.emoji[index] = em;
					let button = d.querySelector("div.mainOption[index='" + index + "'] > div.mainEmoji");
					button.innerHTML = emoji[em];
				};

			for (let i = 0; i < emoji.length; i++)
			{
				let flag = (Buttons.tmp.emoji[indexButton] == i);
				let item = ui.createListItem(emoji[i], i, flag);
				item.addEventListener('click', onchange);
				list.append(item);
			}

			ui.createList(list, true);
		};

	ui.onButtonMelody =
		function ()
		{
			let indexButton = parseInt(this.parentNode.getAttribute('index'));
			let list = d.createElement('div');

			list.setAttribute('value', indexButton);

			var onchange =
				function (e)
				{
					e.stopPropagation();
					ui.createListClose();
					let index = parseInt(this.parentNode.getAttribute('value'))
					let mld = parseInt(this.getAttribute('value'));
					Buttons.tmp.mld[index] = mld;
					let button = d.querySelector("div.mainOption[index='" + index + "'] > div.mainMelody");
					button.classList.removeRegExp('str.+');
					if (mld == -1)
					{
						button.classList.add('strNoSelect');
					}
					else
					{
						button.setAttribute('string', ui.getMelodyName(mld));
						button.classList.add('strAttr');
					}
				};

			let item = ui.createListItem(null, -1, (Buttons.tmp.mld[indexButton] == -1), 'strNoSelect');
			item.addEventListener('click', onchange);
			list.append(item);

			for (let mld of d.querySelectorAll('[id^=melody]'))
			{
				let index = mld.id.match(/^melody(\d+)/);
				if (index)
				{
					index = parseInt(index[1]);
					let flag = (Buttons.tmp.mld[indexButton] == index);
					let text = mld.querySelector('p').innerHTML;
					let item = ui.createListItem(text, index, flag);
					item.addEventListener('click', onchange);
					list.append(item);
				}
			}

			ui.createList(list);
		};

	function createButtonMain(index)
	{
		let item = d.createElement('div');
		let e = d.createElement('div');
		let m = d.createElement('div');
		let mld = Buttons.mld[index];

		item.classList.add('mainOption');
		item.setAttribute('index', index);

		e.classList.add('mainEmoji');
		e.innerHTML = emoji[Buttons.emoji[index]];
		e.addEventListener('click', ui.onButtonEmoji);

		m.classList.add('mainMelody');
		if (mld == -1)
		{
			m.classList.add('strNoSelect');
		}
		else
		{
			let name = ui.getMelodyName(mld);
			if (name)
			{
				m.classList.add('strAttr');
				m.setAttribute('string', name);
			}
			else
			{
				m.classList.add('strNotFound');
			}
		}
		m.addEventListener('click', ui.onButtonMelody);

		item.append(e);
		item.append(m);
		return item;
	}

	ui.updateMainOption =
		function ()
		{
			let mainOption = d.querySelector('div.page[name=MainOption]');
			let i;
			mainOption.innerHTML = '';
			Buttons.tmp.emoji = Buttons.emoji.slice();
			Buttons.tmp.mld = Buttons.mld.slice();
			for (i = 0; i < Buttons.tmp.emoji.length; i++)
			{
				mainOption.append(createButtonMain(i));
			}
		};

	ui.changeMainOption =
		function ()
		{
			Buttons.emoji = Buttons.tmp.emoji.slice();
			Buttons.mld = Buttons.tmp.mld.slice();
		};

	ui.updateMain =
		function ()
		{
			for(let button of d.querySelectorAll('[id^=button]'))
			{
				let index = button.id.match(/^button(\d+)/);
				if (index)
				{
					index = parseInt(index[1]);
					button.innerHTML = emoji[Buttons.emoji[index]];
				}
			}
		};

/* setEvent xxx */

	ui.setEventNameValue =
		function (name)
		{
			let eventName = d.querySelector('#eventName');
			eventName.value = name.toString();
		};

	ui.setEventChannel =
		function (ch)
		{
			let eventChannel = d.querySelector('#eventChannel');
			eventChannel.classList.removeRegExp('ch.+');
			eventChannel.classList.add('ch' + ch);
			eventChannel.setAttribute('channel', ch);
		};

	ui.setEventType =
		function (type)
		{
			let eventType = d.querySelector('#eventType');
			let eventPulses1 = d.querySelector('#eventPulses1');
			eventType.classList.removeRegExp('str.+');
			eventType.setAttribute('type', type);
			eventPulses1.classList.remove('show');
			switch (type)
			{
				case SirenaPulse.Press:
					eventType.classList.add('strPress');
					break;
				case SirenaPulse.Cyclic:
					eventType.classList.add('strCyclic');
					break;
				case SirenaPulse.Pulse:
					eventPulses1.classList.add('show');
					eventType.classList.add('strPulse');
					break;
			}
		};

	ui.setEventMelody =
		function (mld)
		{
			let eventMelody = d.querySelector('#eventMelody');
			eventMelody.classList.removeRegExp('str.+');
			eventMelody.setAttribute('melody', mld);
			if (mld == -1)
			{
				eventMelody.classList.add('strNoSelect');
			}
			else
			{
				let mldName = ui.getMelodyName(mld);
				if (mldName)
				{
					eventMelody.setAttribute('string', mldName);
					eventMelody.classList.add('strAttr');
				}
				else
				{
					eventMelody.classList.add('strNotFound');
				}
			}
		};

	ui.setEventPoweroff =
		function (enable)
		{
			let eventPoweroff = d.querySelector('#eventPoweroff');
			eventPoweroff.checked = enable;
		};

	ui.setEventTypePulse2 =
		function (enable)
		{
			let eventTypePulse2 = d.querySelector('#eventTypePulse2');
			let eventPulses2 = d.querySelector('#eventPulses2');
			eventTypePulse2.checked = enable;
			if (enable)
			{
				eventPulses2.classList.add('show');
			}
			else
			{
				eventPulses2.classList.remove('show');
				ui.setEventTypePulse3(false);
			}
		};

	ui.setEventTypePulse3 =
		function (enable)
		{
			let eventTypePulse3 = d.querySelector('#eventTypePulse3');
			let eventPulses3 = d.querySelector('#eventPulses3');
			eventTypePulse3.checked = enable;
			if (enable)
			{
				eventPulses3.classList.add('show');
				ui.setEventTypePulse2(true);
			}
			else
			{
				eventPulses3.classList.remove('show');
			}
		};

	ui.setEventWidth1 =
		function (value)
		{
			let eventWidth1 = d.querySelector('#eventWidth1');
			eventWidth1.value = value;
		};

	ui.setEventCount1 =
		function (value)
		{
			let eventCount1 = d.querySelector('#eventCount1');
			eventCount1.value = value;
		};

	ui.setEventWidth2 =
		function (value)
		{
			let eventWidth2 = d.querySelector('#eventWidth2');
			eventWidth2.value = value;
		};

	ui.setEventCount2 =
		function (value)
		{
			let eventCount2 = d.querySelector('#eventCount2');
			eventCount2.value = value;
		};

	ui.setEventWidth3 =
		function (value)
		{
			let eventWidth3 = d.querySelector('#eventWidth3');
			eventWidth3.value = value;
		};

	ui.setEventCount3 =
		function (value)
		{
			let eventCount3 = d.querySelector('#eventCount3');
			eventCount3.value = value;
		};

	ui.setEventVolume =
		function (value)
		{
			let eventVolume = d.querySelector('#eventVolume');
			eventVolume.value = value;
		};

	ui.setEventNoPlayAfter =
		function (evt)
		{
			let eventNoPlayAfter = d.querySelector('#eventNoPlayAfter');
			eventNoPlayAfter.classList.removeRegExp('str.+');
			eventNoPlayAfter.setAttribute('event', evt);
			if (evt == -1)
			{
				eventNoPlayAfter.classList.add('strNoPlayAfterNull');
			}
			else
			{
				let evtName = ui.getNoPlayAfterName(evt);
				if (evtName)
				{
					eventNoPlayAfter.setAttribute('string', evtName);
					eventNoPlayAfter.classList.add('strAttr');
				}
				else
				{
					eventNoPlayAfter.classList.add('strNotFound');
				}
			}
		};

/* ----------------------------------------- */

/* getEvent xxx */

	ui.getEventNameValue =
		function ()
		{
			let eventName = d.querySelector('#eventName');
			return eventName.value.trim();
		};

	ui.getEventChannel =
		function ()
		{
			let eventChannel = d.querySelector('#eventChannel');
			return parseInt(eventChannel.getAttribute('channel'));
		};

	ui.getEventType =
		function ()
		{
			let eventType = d.querySelector('#eventType');
			return parseInt(eventType.getAttribute('type'));
		};

	ui.getEventMelody =
		function ()
		{
			let eventMelody = d.querySelector('#eventMelody');
			return parseInt(eventMelody.getAttribute('melody'));
		};

	ui.getEventPoweroff =
		function ()
		{
			let eventPoweroff = d.querySelector('#eventPoweroff');
			return eventPoweroff.checked;
		};

	ui.getEventTypePulse2 =
		function ()
		{
			let eventTypePulse2 = d.querySelector('#eventTypePulse2');
			return eventTypePulse2.checked;
		};

	ui.getEventTypePulse3 =
		function ()
		{
			let eventTypePulse3 = d.querySelector('#eventTypePulse3');
			return eventTypePulse3.checked;
		};

	ui.getEventWidth1 =
		function ()
		{
			let eventWidth1 = d.querySelector('#eventWidth1');
			let min = parseInt(eventWidth1.getAttribute('min'));
			let max = parseInt(eventWidth1.getAttribute('max'));
			let value = parseInt(eventWidth1.value);
			if (isNaN(value)) value = max;
			if (min > value) value = min;
			if (max < value) value = max;
			return value;
		};

	ui.getEventCount1 =
		function ()
		{
			let eventCount1 = d.querySelector('#eventCount1');
			let min = parseInt(eventCount1.getAttribute('min'));
			let max = parseInt(eventCount1.getAttribute('max'));
			let value = parseInt(eventCount1.value);
			if (isNaN(value)) value = max;
			if (min > value) value = min;
			if (max < value) value = max;
			return value;
		};

	ui.getEventWidth2 =
		function ()
		{
			let eventWidth2 = d.querySelector('#eventWidth2');
			let min = parseInt(eventWidth2.getAttribute('min'));
			let max = parseInt(eventWidth2.getAttribute('max'));
			let value = parseInt(eventWidth2.value);
			if (isNaN(value)) value = max;
			if (min > value) value = min;
			if (max < value) value = max;
			return value;
		};

	ui.getEventCount2 =
		function ()
		{
			let eventCount2 = d.querySelector('#eventCount2');
			let min = parseInt(eventCount2.getAttribute('min'));
			let max = parseInt(eventCount2.getAttribute('max'));
			let value = parseInt(eventCount2.value);
			if (isNaN(value)) value = max;
			if (min > value) value = min;
			if (max < value) value = max;
			return value;
		};

	ui.getEventWidth3 =
		function ()
		{
			let eventWidth3 = d.querySelector('#eventWidth3');
			let min = parseInt(eventWidth3.getAttribute('min'));
			let max = parseInt(eventWidth3.getAttribute('max'));
			let value = parseInt(eventWidth3.value);
			if (isNaN(value)) value = max;
			if (min > value) value = min;
			if (max < value) value = max;
			return value;
		};

	ui.getEventCount3 =
		function ()
		{
			let eventCount3 = d.querySelector('#eventCount3');
			let min = parseInt(eventCount3.getAttribute('min'));
			let max = parseInt(eventCount3.getAttribute('max'));
			let value = parseInt(eventCount3.value);
			if (isNaN(value)) value = max;
			if (min > value) value = min;
			if (max < value) value = max;
			return value;
		};

	ui.getEventVolume =
		function ()
		{
			let eventCount3 = d.querySelector('#eventVolume');
			let min = parseInt(eventCount3.getAttribute('min'));
			let max = parseInt(eventCount3.getAttribute('max'));
			let value = parseInt(eventCount3.value);
			if (isNaN(value)) value = max;
			if (min > value) value = min;
			if (max < value) value = max;
			return value;
		};

	ui.getEventNoPlayAfter =
		function ()
		{
			let eventNoPlayAfter = d.querySelector('#eventNoPlayAfter');
			return parseInt(eventNoPlayAfter.getAttribute('event'));
		};

/* ----------------------------------------- */

	ui.updateEventAdd =
		function ()
		{
			if (vars.eventEdit == -1)
			{
				if (Events.length >= vars.eventMax)
				{
					ui.onClickMenuItem('Event');
					ui.Message('strEventLimit');
				}
				else
				{
					ui.setEventNameValue('');
					ui.setEventChannel(1);
					ui.setEventType(SirenaPulse.Press);
					ui.setEventMelody(-1);
					ui.setEventPoweroff(true);
					ui.setEventTypePulse2(false);
					ui.setEventWidth1(100);
					ui.setEventCount1(1);
					ui.setEventWidth2(300);
					ui.setEventCount2(1);
					ui.setEventWidth3(600);
					ui.setEventCount3(1);
					ui.setEventVolume(8);
					ui.setEventNoPlayAfter(-1);
					if (vars.eventFromLog)
					{
						ui.setEventChannel(vars.eventFromLog.chIndex);
						ui.setEventType(vars.eventFromLog.type);
						if (vars.eventFromLog.type == SirenaPulse.Pulse)
						{
							if (vars.eventFromLog.pulse.length >= 1)
							{
								ui.setEventWidth1(vars.eventFromLog.pulse[0].width);
								ui.setEventCount1(vars.eventFromLog.pulse[0].count);
							}
							if (vars.eventFromLog.pulse.length >= 2)
							{
								ui.setEventTypePulse2(true);
								ui.setEventWidth2(vars.eventFromLog.pulse[1].width);
								ui.setEventCount2(vars.eventFromLog.pulse[1].count);
							}
							if (vars.build < 1590 && vars.eventFromLog.pulse.length >= 3)
							{
								ui.setEventTypePulse3(true);
								ui.setEventWidth3(vars.eventFromLog.pulse[2].width);
								ui.setEventCount3(vars.eventFromLog.pulse[2].count);
							}
						}
						vars.eventFromLog = null;
					}
				}
			}
			else
			{
				for (let i = 0; i < Events.length; i++)
				{
					let event = Events[i];
					if (event.index == vars.eventEdit)
					{
						vars.eventEdit = i;
						ui.setEventNameValue(ui.findEventName(event.index));
						ui.setEventChannel(event.chIndex);
						ui.setEventType(event.type);
						ui.setEventMelody(event.melodyIndex);
						ui.setEventPoweroff(event.poweroff);
						if (vars.build >= 1513) ui.setEventVolume(event.volume);
						if (vars.build >= 1590) ui.setEventNoPlayAfter(event.noPlayAfter);
						ui.setEventTypePulse2(false);
						ui.setEventWidth1(100);
						ui.setEventCount1(1);
						ui.setEventWidth2(300);
						ui.setEventCount2(1);
						ui.setEventWidth3(600);
						ui.setEventCount3(1);
						if (event.type == SirenaPulse.Pulse)
						{
							if (event.pulse.length >= 1)
							{
								ui.setEventWidth1(event.pulse[0].width);
								ui.setEventCount1(event.pulse[0].count);
							}
							if (event.pulse.length >= 2)
							{
								ui.setEventTypePulse2(true);
								ui.setEventWidth2(event.pulse[1].width);
								ui.setEventCount2(event.pulse[1].count);
							}
							if (vars.build < 1590 && event.pulse.length >= 3)
							{
								ui.setEventTypePulse3(true);
								ui.setEventWidth3(event.pulse[2].width);
								ui.setEventCount3(event.pulse[2].count);
							}
						}
						break;
					}
				}
			}
		};

	ui.eventSelectChannel =
		function ()
		{
			const channels = [1, 2, 3];
			let list = d.createElement('div');
			let currentChannel = ui.getEventChannel();
			let onchange =
				function (e)
				{
					e.stopPropagation();
					ui.createListClose();
					let ch = parseInt(this.getAttribute('value'));
					ui.setEventChannel(ch);
				};
			for (let i of channels)
			{
				let item = ui.createListItem(' ' + i, i, (i == currentChannel), 'strChannel');
				item.addEventListener('click', onchange);
				list.append(item);
			}
			ui.createList(list);
		};

	ui.eventSelectType =
		function ()
		{
			const types = [SirenaPulse.Press, SirenaPulse.Pulse, SirenaPulse.Cyclic];
			let list = d.createElement('div');
			let currentType = ui.getEventType();
			let onchange =
				function (e)
				{
					e.stopPropagation();
					ui.createListClose();
					let type = parseInt(this.getAttribute('value'));
					ui.setEventType(type);
				};
			for (let i of types)
			{
				let cls;
				switch (i)
				{
					case SirenaPulse.Press:		cls = 'strPress';break;
					case SirenaPulse.Pulse:		cls = 'strPulse';break;
					case SirenaPulse.Cyclic:	cls = 'strCyclic';break;
				}
				let item = ui.createListItem(null, i, (i == currentType), cls);
				item.addEventListener('click', onchange);
				list.append(item);
			}
			ui.createList(list);
		};

	ui.eventSelectMelody =
		function ()
		{
			let list = d.createElement('div');
			let mldId = ui.getEventMelody();
			let item = ui.createListItem(null, -1, (mldId == -1), 'strNoSelect');
			let onchange =
				function (e)
				{
					e.stopPropagation();
					ui.createListClose();
					let index = parseInt(this.getAttribute('value'));
					ui.setEventMelody(index);
				};
			item.addEventListener('click', onchange);
			list.append(item);
			for (let mld of d.querySelectorAll('[id^=melody]'))
			{
				let index = mld.id.match(/^melody(\d+)/);
				if (index)
				{
					index = parseInt(index[1]);
					let flag = (mldId == index);
					let text = mld.querySelector('p').innerHTML;
					let item = ui.createListItem(text, index, flag);
					item.addEventListener('click', onchange);
					list.append(item);
				}
			}
			ui.createList(list);
		};

	ui.eventSelectNoPlayAfter =
		function ()
		{
			let list = d.createElement('div');
			let evtId = ui.getEventNoPlayAfter();
			let item = ui.createListItem(null, -1, (evtId == -1), 'strNoPlayAfterNull');
			let onchange =
				function (e)
				{
					e.stopPropagation();
					ui.createListClose();
					let index = parseInt(this.getAttribute('value'));
					ui.setEventNoPlayAfter(index);
				};
			item.addEventListener('click', onchange);
			list.append(item);
			for (let evt of d.querySelectorAll('[id^=event]'))
			{
				let index = evt.id.match(/^event(\d+)/);
				if (index)
				{
					index = parseInt(index[1]);
					if (index != vars.eventEdit)
					{
						let flag = (evtId == index);
						let text = ui.getNoPlayAfterName(index);
						let item = ui.createListItem(text, index, flag);
						item.addEventListener('click', onchange);
						list.append(item);
					}
				}
			}
			ui.createList(list);
		};

	ui.eventApplyValue =
		function ()
		{
			let event = ((vars.eventEdit == -1) ? {} : Events[vars.eventEdit]);
			event.chIndex		= ui.getEventChannel();
			event.melodyIndex	= ui.getEventMelody();
			event.poweroff		= ui.getEventPoweroff();
			event.volume		= ui.getEventVolume();
			event.noPlayAfter	= ui.getEventNoPlayAfter();
			event.type			= ui.getEventType();
			event.pulse			= [];
			if (event.type == SirenaPulse.Pulse)
			{
				event.pulse.push({ width : ui.getEventWidth1(), count : ui.getEventCount1() });
				if (ui.getEventTypePulse2())
				{
					event.pulse.push({ width : ui.getEventWidth2(), count : ui.getEventCount2() });
					if (ui.getEventTypePulse3())
					{
						event.pulse.push({ width : ui.getEventWidth3(), count : ui.getEventCount3() });
					}
				}
			}
			if (vars.eventEdit == -1)
			{
				event.index = -1;
				for (let i = 0; i < vars.eventMax; i++)
				{
					let flag = true;
					for (let e of Events)
					{
						if (e.index == i)
						{
							flag = false;
							break;
						}
					}
					if (flag)
					{
						event.index = i;
						break;
					}
				}
				Events = Events.concat([event]);
			}
			eventName[event.index] = ui.getEventNameValue();
			ui.setEventName();
			jsi.setEvent(JSON.stringify(event));
		};

	ui.onContextMenuLog =
		function (e)
		{
			let items = d.createElement('div');
			let item;
			var evt = this;
			item = d.createElement('div');
			item.classList.add('strClear');
			item.addEventListener('click',
				function (e)
				{
					ui.onContextMenuClose(e);
					d.querySelector('div.page[name=Log]').innerHTML = '';
				});
			items.append(item);
			ui.onContextMenu.call(this, e, null, items);
		};

	ui.onSetConfig =
		function (e)
		{
			e.stopPropagation();
			let measure = JSON.parse(this.getAttribute('measure'));
			let ch0PressMax = d.querySelector('#ch0PressMax');
			let ch0IdleMax = d.querySelector('#ch0IdleMax');
			ch0PressMax.value = measure.width;
			ch0IdleMax.value = measure.idle;
			ch0PressMax.onchange.call(ch0PressMax);
			ch0IdleMax.onchange.call(ch0IdleMax);
			ui.Message('strSetConfig');
		};

	ui.onAddEvent =
		function (e)
		{
			e.stopPropagation();
			let chIndex = parseInt(this.getAttribute('chIndex'));
			let type = parseInt(this.getAttribute('type'));
			let pulse = this.getAttribute('pulse');
			let event = {};
			event.chIndex = chIndex;
			event.type = type;
			if (pulse)
			{
				event.pulse = JSON.parse(pulse);
			}
			vars.eventFromLog = event;
			ui.onClickMenuItem('EventAdd');
		};

	ui.cbEvent =
		function ()
		{
			let event = jsi.getLogEvent();
			if (event)
			{
				event = JSON.parse(event);
				let log = d.querySelector('div.page[name=Log]');
				let item = d.createElement('div');
				let left = d.createElement('div');
				let center = d.createElement('div');
				let right = d.createElement('div');
				let channel = d.createElement('div');
				let channelP = d.createElement('p');
				let typeP = d.createElement('p');

				item.classList.add('logItem');
				left.classList.add('left');
				center.classList.add('center');
				right.classList.add('right');

				channel.classList.add('channel');
				channel.classList.add('ch' + event.chIndex);
				channelP.innerHTML = event.chIndex;

				switch (event.type)
				{
					case SirenaPulse.Press:
						typeP.classList.add('strPress');
						break;
					case SirenaPulse.Release:
						typeP.classList.add('strRelease');
						break;
					case SirenaPulse.Cyclic:
						typeP.classList.add('strCyclic');
						break;
					case SirenaPulse.Pulse:
						typeP.classList.add('strPulse');
						let flag = false;
						var s = ' ';
						event.pulse.forEach(
							function (pulse)
							{
								if (flag) s += ', '; else flag = true;
								s += pulse.width + ':' + pulse.count;
							});
						typeP.innerHTML = s;
						break;
					case SirenaPulse.Measure:
						typeP.classList.add('strMeasure');
						typeP.innerHTML = ' ' + event.measure.width + '/' + event.measure.idle;
						break;
					case SirenaPulse.MeasureError:
						typeP.classList.add('strMeasureError');
						break;
				}

				if (event.type == SirenaPulse.Press || event.type == SirenaPulse.Cyclic || event.type == SirenaPulse.Pulse)
				{
					right.classList.add('imgAdd2');
					right.setAttribute('chIndex', event.chIndex);
					right.setAttribute('type', event.type);
					if (event.pulse !== undefined)
					{
						right.setAttribute('pulse', JSON.stringify(event.pulse));
					}
					right.addEventListener('click', ui.onAddEvent);
				}
				else if (event.type == SirenaPulse.Measure)
				{
					right.classList.add('imgEdit2');
					right.setAttribute('type', event.type);
					right.setAttribute('measure', JSON.stringify(event.measure));
					right.addEventListener('click', ui.onSetConfig);
				}

				channel.append(d.createElement('div'));
				channel.append(channelP);
				left.append(channel);
				center.append(typeP);
				item.append(left);
				item.append(center);
				item.append(right);

				log.insertBefore(item, log.firstChild);
				while (log.childNodes.length > 30)
				{
					log.lastChild.remove();
				}
			}
		};

	ui.cbSetMelody =
		function (index)
		{
			let mld = { 'index' : index, name : vars.pack.name };
			let progress = d.querySelector('#melodyPercent');
			let page = d.querySelector('div.page[name=Melody]');
			progress.value = 0;
			progress.parentNode.classList.add('show');
			page.append(createMelody(mld));
            ui.headerTextSubMelodyTotal();
		};

	ui.cbProgressMelody =
		function (percent)
		{
			d.querySelector('#melodyPercent').value = percent;
		};

	ui.cbEndMelody =
		function ()
		{
			let progress = d.querySelector('div.progress');
			if (progress) progress.classList.remove('show');
		};

	ui.cbErrorMelody =
		function ()
		{
			d.querySelector('div.progress').classList.remove('show');
			ui.Message('strErrorLoad');
		};

	function onResizeMain()
	{
		let width = w.innerWidth;
		let height = w.innerHeight - 64;
		let size = ((width > height) ? height / 3 : width / 3);
		for(let item of d.querySelectorAll('.page[name=Main] > div > div'))
		{
			item.style.width = size + 'px';
			item.style.height = size + 'px';
			item.style.fontSize = (size / 2) + 'px';
		}
	}

	function onResizeMenu()
	{
		let menu = d.querySelector('.menu');
		if (menu)
		{
			let styleMenu = d.querySelector('#styleMenu');
			styleMenu.innerHTML = '.menu{left:-' + menu.scrollWidth + 'px;}';
		}
	}

	function onResize()
	{
		let height = w.innerHeight + 'px';
		d.body.style.height = height;
		onResizeMain();
		onResizeMenu();
	}

	function onContextMenuMelody(e)
	{
		let items = d.createElement('div');
		let item;
		var mld = this;
		item = d.createElement('div');
		item.classList.add('strPlay');
		item.addEventListener('click',
			function (e)
			{
				ui.onContextMenuClose(e);
				let mldId = parseInt(mld.parentNode.id.match(/^melody(\d+)/)[1]);
				jsi.play(mldId);
			});
		items.append(item);
		item = d.createElement('div');
		item.classList.add('strDelete');
		item.addEventListener('click',
			function (e)
			{
				ui.onContextMenuClose(e);
				let mldId = parseInt(mld.parentNode.id.toString().match(/^melody(\d+)/)[1]);
				jsi.deleteMelody(mldId);
			});
		items.append(item);
		ui.onContextMenu.call(this, e, '#' + mld.parentNode.id, items);
	}

	function createMelody(mld)
	{
		let item = d.createElement('div');
		let p = d.createElement('p');
		let div = d.createElement('div');

		item.classList.add('loadedMelody');
		item.id = 'melody' + mld.index;

		p.innerHTML = mld.name;

		div.classList.add('imgSubmenu2');
		div.addEventListener('click', onContextMenuMelody);

		item.append(p);
		item.append(div);

		vars.melodys++;

		return item;
	}

	function onContextMenuEvent(e)
	{
		let items = d.createElement('div');
		let item;
		var evt = this;
		item = d.createElement('div');
		item.classList.add('strChange');
		item.addEventListener('click',
			function (e)
			{
				ui.onContextMenuClose(e);
				let evtId = parseInt(evt.parentNode.id.match(/^event(\d+)/)[1]);
				vars.eventEdit = evtId;
				ui.onClickMenuItem('EventAdd');
			});
		items.append(item);
		item = d.createElement('div');
		item.classList.add('strDelete');
		item.addEventListener('click',
			function (e)
			{
				ui.onContextMenuClose(e);
				let evtId = parseInt(evt.parentNode.id.match(/^event(\d+)/)[1]);
				jsi.deleteEvent(evtId);
			});
		items.append(item);
		ui.onContextMenu.call(this, e, '#' + evt.parentNode.id, items);
	}

	function createEvent(evt)
	{
		let item = d.createElement('div');
		let left = d.createElement('div');
		let center = d.createElement('div');
		let right = d.createElement('div');
		let channel = d.createElement('div');
		let channelBG = d.createElement('div');
		let channelN = d.createElement('p');
		let off = d.createElement('div');
		let type = d.createElement('p');
		let melody = d.createElement('p');
		let name = d.createElement('p');

		item.setAttribute('id', 'event' + evt.index);
		item.classList.add('configuredEvent');
		if (vars.build >=1590 && evt.noPlayAfter != -1)
		{
			item.classList.add('noPlayAfter');
		}

		left.classList.add('left');
		center.classList.add('center');
		right.classList.add('right');
		right.classList.add('imgSubmenu2');
		right.addEventListener('click', onContextMenuEvent);

		channel.classList.add('channel');
		channel.classList.add('ch' + evt.chIndex);
		channelN.innerHTML = evt.chIndex;

		off.classList.add('imgOff');
		if (evt.poweroff) off.classList.add('show');

		name.classList.add('name');
		name.innerHTML = ui.findEventName(evt.index);

		type.classList.add("type");
		switch (evt.type)
		{
			case SirenaPulse.Press:
				type.classList.add("strPress");
				break;
			case SirenaPulse.Cyclic:
				type.classList.add("strCyclic");
				break;
			case SirenaPulse.Pulse:
				type.classList.add("strPulse");
				let flag = false;
				var s = ' ';
				evt.pulse.forEach(
					function (pulse)
					{
						if (flag) s += ', '; else flag = true;
						s += pulse.width + ':' + pulse.count;
					});
				type.innerHTML = s;
				break;
		}

		melody.classList.add("melody");
		if (evt.melodyIndex == -1)
		{
			melody.classList.add("strNoSelect");
		}
		else
		{
			let mldName = ui.getMelodyName(evt.melodyIndex);
			if (mldName)
			{
				melody.setAttribute('string', mldName);
				melody.classList.add("strAttr");
			}
			else
			{
				melody.classList.add("strNotFound");
			}
		}

		channel.append(channelBG);
		channel.append(channelN);
		left.append(channel);
		left.append(off);
		center.append(name);
		center.append(type);
		center.append(melody);
		item.append(left);
		item.append(center);
		item.append(right);
		return item;
	}

	ui.updateEvent =
		function ()
		{
			let page = d.querySelector('div.page[name=Event]');
			page.innerHTML = '';
			vars.eventEdit = -1;
			if (Events != null)
			{
				Events.forEach(
					function (evt)
					{
						page.append(createEvent(evt));
					});
			}
			ui.headerTextSubEventTotal();
		};

	function setLang()
	{
		let lang = 'en';
		let _lang = (w.jsi !== undefined) ? jsi.getLang().toLowerCase() : '';
		switch (_lang)
		{
			case 'ru':
			case 'en':
				lang = _lang;
				break;
			case 'by':
			case 'kz':
			case 'ua':
			case 'uz':
				lang = 'ru';
				break;
		}
		d.querySelector('html').setAttribute('lang', lang);
		//onResizeMenu();
	}

	function getConfig(callback)
	{
		let config = jsi.getConfig();
		if (config)
		{
			config = JSON.parse(config);

			vars.version = config.version;
			vars.build = config.build;
			vars.melodyMax = config.melodyMax;
			vars.eventMax = config.eventMax;
			vars.serial = config.serial;

			if (vars.build >= 1513) d.body.classList.add('b1513');
			if (vars.build >= 1590) d.body.classList.add('b1590');
			if (vars.build >= 1722) d.body.classList.add('b1722');
			if (vars.build >= 1778) d.body.classList.add('b1778');

			let page = d.querySelector('div.page[name=Melody]');
			page.innerHTML = '';
			config.melodys.forEach(
				function (mld)
				{
					page.append(createMelody(mld));
				});

			Events = config.events;

			d.querySelector('#ch0PressMax').value		= config.channels[0].pressMax;
			d.querySelector('#ch0IdleMax').value		= config.channels[0].idleMax;
			d.querySelector('#ch0PulsesMax').value		= config.channels[0].pulsesMax;
			d.querySelector('#ch0Deviation').value		= config.channels[0].deviation;
			//d.querySelector('#ch0CHButton').checked		= config.channels[0].button;
			d.querySelector('#ch0CHMeasure').checked	= config.channels[0].measure;

			if (config.build >= 1331)
			{
				d.querySelector('#options1331').classList.add('show');
				for (let i = 0; i < 3; i++)
				{
					d.querySelector('#ch' + i + 'CHButton').checked		= config.channels[i].button;
					d.querySelector('#ch' + i + 'PlayOnce').checked		= config.channels[i].playOnce;
					d.querySelector('#ch' + i + 'PlayToEnd').checked	= config.channels[i].playToEnd;
				}
				d.querySelector('#ch0NoWidth').checked = config.channels[0].noWidth;
			}
			else
			{
				d.querySelector('#ch0CHButton').checked = config.channels[0].button;
			}

			if (config.build >= 1513)
			{
				d.querySelector('#ch0NoBounceHigh').checked = config.channels[0].noBounceHigh;
			}

			if (config.build >= 1778)
			{
				for (let i = 0; i < 2; i++)
				{
					d.querySelector('#ch' + i + 'Standard').checked		= config.channels[i].standard;
				}
				d.querySelector('#ch1PressMax').value		= config.channels[1].pressMax;
				d.querySelector('#ch1IdleMax').value		= config.channels[1].idleMax;
				d.querySelector('#ch1PulsesMax').value		= config.channels[1].pulsesMax;
				d.querySelector('#ch1Deviation').value		= config.channels[1].deviation;
				d.querySelector('#ch1CHMeasure').checked	= config.channels[1].measure;
				d.querySelector('#ch1NoWidth').checked 		= config.channels[1].noWidth;
				d.querySelector('#ch1NoBounceHigh').checked = config.channels[1].noBounceHigh;
			}

			/* 1722 */
			d.querySelector('#cfgVolume').value = config.volume;
			d.querySelector('#cfgVolumeBeep').value = config.volumeBeep;
			d.querySelector('#cfgSleep').checked = config.sleep;

			d.querySelector('#PINCode').value = config.pincode;

			d.querySelector('div.waitPage').classList.remove('show');

			ui.updateMain();

			if (config.build < 1777)
			{
				let updateMessage =
					function ()
					{
						if (ui.isPage('Update'))
						{
							setTimeout(updateMessage, 1000);
						}
						else
						{
							ui.Message('strUpdate1777', true);
							setTimeout(updateMessage, 8000);
						}
					};
				setTimeout(updateMessage, 3000);
			}
			/*else
			{
				let serialCheck =
					function () 
					{
						var xhr = new XMLHttpRequest();
						xhr.addEventListener('loadend',
							function (e)
							{
								if (xhr.status == 200)
								{
									jsi.checkSerial(xhr.responseText);
								}
								else
								{
									ui.Message('strInternetError', true);
									setTimeout(serialCheck, 4000);
								}
							});
						xhr.open('POST', apiUpdateURL() + 'serial/');
						let form = new FormData();
						form.append('serial', vars.serial);
						xhr.send(form);
					};
				serialCheck();
			}*/

			callback();
		}
		else
		{
			setTimeout(getConfig, 1000, callback);
		}
	}

	ui.updateSelectChange =
		function ()
		{
			let buildInfo = d.querySelector('div.page[name=Update] > p.buildInfo');
			let option = d.querySelector('div.page[name=Update] > select.builds > option:checked');
			if (option)
			{
				buildInfo.innerHTML = option.getAttribute('data-info');
			}
		};

	ui.updateUpdate =
		function ()
		{
			var url = apiUpdateURL();
			var current = d.querySelector('div.page[name=Update] > p.current');
			var update = d.querySelector('div.page[name=Update] > p.update');
			var builds = d.querySelector('div.page[name=Update] > select.builds');
			var button = d.querySelector('div.page[name=Update] > button');
			var warning = d.querySelector('div.page[name=Update] > p.strUpdateWarning');
			var buildInfo = d.querySelector('div.page[name=Update] > p.buildInfo');
			current.innerHTML = vars.version.toFixed(1) + '.' + vars.build;
			update.classList.add('strUpdateCheck');
			update.classList.remove('strUpdateCheckError');
			button.classList.remove('show');
			warning.classList.remove('show');
			buildInfo.innerHTML = '';
			let child;
			while (child = builds.lastChild)
			{
				builds.remove(child);
			}
			var xhr = new XMLHttpRequest();
			xhr.addEventListener('loadend',
				function (e)
				{
					if (xhr.status == 200)
					{
						update.classList.remove('strUpdateCheck');
						let response = JSON.parse(xhr.responseText);
						Object.keys(response.builds).forEach(
							function (k)
							{
								let o = d.createElement('OPTION');
								o.value = k;
								o.innerHTML = k + ' ' + response.builds[k].type;
								o.setAttribute('data-url', response.builds[k].url);
								o.setAttribute('data-info', response.builds[k].info);
								if (k == response.build) o.selected = true;
								builds.append(o);
							});
						button.classList.add('show');
						warning.classList.add('show');
						ui.updateSelectChange();
					}
					else
					{
						update.classList.remove('strUpdateCheck');
						update.classList.add('strUpdateCheckError');
					}
				});
			xhr.open('GET', url);
			xhr.send();
		};

	ui.updateStart =
		function ()
		{
			let option = d.querySelector('div.page[name=Update] > select.builds > option:checked');
			if (!option) return;
			if (parseInt(option.value) == vars.build)
			{
				ui.Message('strUpdateIsSame');
				return;
			}
			this.disabled = true;
			var url = option.getAttribute('data-url');
			var xhr = new XMLHttpRequest();
			xhr.btn = this;
			xhr.addEventListener('loadend',
				function (e)
				{
					if (xhr.status == 200)
					{
						let response = JSON.parse(xhr.responseText);
						if (!jsi.setUpdate(response.fw, response.hash))
						{
							ui.Message('strUpdateLoadBad');
							xhr.btn.disabled = false;
						}
					}
					else
					{
						ui.Message('strUpdateLoadError');
						xhr.btn.disabled = false;
					}
				});
			xhr.open('GET', url);
			xhr.send();
		};

	function getListFiles()
	{
		
		var remote = {};
		if (vars.pack.local)
		{
			remote.catalog = [];
			remote.files = [];
			let downloads = JSON.parse(jsi.getDownloadFiles());
			downloads.files.forEach(
				function (file)
				{
					let obj = {};
					obj.name = file.replace(/\.wav$/i, '');
					obj.file = downloads.catalog + '/' + file;
					remote.files.push(obj);
				});
		}
		else
		{
			let current = vars.pack.current.split('/');
			remote = vars.pack.remote;
			if (remote)
			{
				for (let i = 0; i < current.length; i++)
				{
					remote = remote.catalog[parseInt(current[i])];
				}
			}
		}
		return remote;
	}

	var nextID = 0;
	function incNextID()
	{
		nextID++;
		if (nextID > 1000000) nextID = 0;
	}

	function createCatalog(index, catalog)
	{
		let group = d.createElement('div');
		let item = d.createElement('label');
		let img = d.createElement('div');
		let p = d.createElement('p');
		let onselect =
			function (e)
			{
				e.stopPropagation();
				let index = parseInt(this.getAttribute('index'));
				if (index == -1)
				{
					if (vars.pack.local)
					{
						vars.pack.local = false;
					}
					else
					{
						let current = vars.pack.current.split('/')
						current.pop();
						vars.pack.current = current.join('/');
					}
				}
				else if (index == -2)
				{
					vars.pack.local = true;
				}
				else
				{
					vars.pack.current += '/' + index;
				}
				ui.updateMelodyAddCurrent();
			};
		group.addEventListener('click', onselect);
		group.setAttribute('index', index);
		img.classList.add('imgCatalog2');
		if (catalog)
		{
			if (typeof(catalog) == 'string')
			{
				let span = d.createElement('span');
				span.classList.add(catalog);
				p.append(span);
			}
			else
			{
				p.innerHTML = catalog.name;
			}
		}
		item.append(img);
		item.append(p);
		group.append(item);
		incNextID();
		return group;
	}

	function selectFile(file)
	{
		let submenu = d.querySelector('#buttonSubmenu');
		if (file)
		{
			vars.pack.file = file;
			vars.pack.name = file.match(/([^\/]+)\.wav$/i)[1];
			submenu.classList.add('imgOk');
		}
		else
		{
			vars.pack.file = null;
			submenu.classList.remove('imgOk');
		}
	}

	function createFile(index, file)
	{
		let group = d.createElement('div');
		let input = d.createElement('input');
		let item = d.createElement('label');
		let img = d.createElement('div');
		let p = d.createElement('p');
		let play = d.createElement('div');
		let id = '__file' + nextID;
		let onchange =
			function (e)
			{
				e.stopPropagation();
				selectFile(this.getAttribute('file'));
			};
		let onplay =
			function (e)
			{
				e.stopPropagation();
				if (this.classList.contains('imgPause2'))
				{
					vars.pack.audio.pause();
					vars.pack.audio.onended.call(vars.pack.audio);
				}
				else if (this.classList.contains('imgPlay2'))
				{
					let file = this.parentNode.previousSibling.getAttribute('file');
					if (vars.pack.audio)
					{
						vars.pack.audio.pause();
						vars.pack.audio.onended.call(vars.pack.audio);
					}
					else
					{
						vars.pack.audio = new Audio();
						vars.pack.audio.addEventListener('loadstart',
							function ()
							{
								this.parent.classList.removeRegExp('img.+');
								this.parent.classList.add('imgLoad');
							});
						vars.pack.audio.addEventListener('playing',
							function ()
							{
								this.parent.classList.removeRegExp('img.+');
								this.parent.classList.add('imgPause2');
							});
						vars.pack.audio.addEventListener('error',
							function ()
							{
								this.parent.classList.removeRegExp('img.+');
								this.parent.classList.add('imgPlay2');
								ui.Message('strErrorPlay');
							});
						vars.pack.audio.onended =
							function ()
							{
								this.parent.classList.removeRegExp('img.+');
								this.parent.classList.add('imgPlay2');
							};
					}
					vars.pack.audio.parent = this;
					vars.pack.audio.src = apiMelodyURL() + file;
					vars.pack.audio.play();
				}
			};
		input.setAttribute('id', id);
		input.setAttribute('type', 'radio');
		input.setAttribute('name', 'melody');
		input.setAttribute('file', file.file);
		input.addEventListener('change', onchange);
		item.setAttribute('for', id);
		img.classList.add('imgMelody2');
		p.innerHTML = file.name;
		if (!vars.pack.local)
		{
			play.classList.add('imgPlay2');
			play.addEventListener('click', onplay);
		}
		item.append(img);
		item.append(p);
		item.append(play);
		group.append(input);
		group.append(item);
		incNextID();
		return group;
	}

	ui.startLoadMelody =
		function ()
		{
			d.querySelector('div.waitPage').classList.add('show');
			if (vars.pack.audio)
			{
				vars.pack.audio.pause();
				vars.pack.audio = null;
			}
			setTimeout(
				function ()
				{
					if (!jsi.startLoadMelody(((vars.pack.local) ? '' : apiMelodyURL()) + vars.pack.file))
					{
						ui.Message('strErrorFile');
					}
					d.querySelector('div.waitPage').classList.remove('show');
				}, 100);
		};

	ui.updateMelodyAddCurrent =
		function ()
		{
			let items = d.querySelector('div.page[name=MelodyAdd]');
			let remote = getListFiles();

			items.innerHTML = '';
			selectFile();

			if (vars.pack.audio)
			{
				vars.pack.audio.pause();
				vars.pack.audio.onended.call(vars.pack.audio);
			}

			if (vars.pack.local || vars.pack.current != '0')
			{
				items.append(createCatalog(-1));
			}
			else
			{
				items.append(createCatalog(-2, 'strDownloadFiles'));
			}
			if (remote)
			{
				for (let i = 0; i < remote.catalog.length; i++)
				{
					items.append(createCatalog(i, remote.catalog[i]));
				}

				for (let i = 0; i < remote.files.length; i++)
				{
					items.append(createFile(i, remote.files[i]));
				}
			}
			else
			{
				let p = d.createElement('p');
				p.classList.add('strUpdateCheckError');
				items.append(p);
			}
		};

	ui.updateMelodyAdd =
		function ()
		{
			if (vars.melodys >= vars.melodyMax)
			{
				ui.onClickMenuItem('Melody');
				ui.Message('strMelodyLimit');
				return;
			}
			if (vars.pack.remote)
			{
				ui.updateMelodyAddCurrent();
			}
			else
			{
				d.querySelector('div.waitPage').classList.add('show');
				var url = apiMelodyURL();
				var xhr = new XMLHttpRequest();
				xhr.addEventListener('loadend',
					function (e)
					{
						vars.pack.current = '0';
						if (xhr.status == 200)
						{
							vars.pack.remote = JSON.parse(xhr.responseText);
						}
						else
						{
							vars.pack.remote = null;
						}
						ui.updateMelodyAddCurrent();
						d.querySelector('div.waitPage').classList.remove('show');
					});
				xhr.open('GET', url);
				xhr.send();
			}
		};

	ui.cbButtons =
	ui.getButtons =
		function ()
		{
			let b = JSON.parse(jsi.getButtons());
			Buttons.emoji = b.emoji;
			Buttons.mld = b.mld;
			ui.updateMain();
		};

	ui.setButtons =
		function ()
		{
			let b = { emoji : Buttons.emoji, mld : Buttons.mld };
			jsi.setButtons(JSON.stringify(b));
		};

	ui.findEventName =
		function (i)
		{
			return (i in eventName) ? eventName[i] : '';
		};

	ui.cbEventName =
	ui.getEventName =
		function ()
		{
			eventName = JSON.parse(jsi.getEventName());
		};

	ui.setEventName =
		function ()
		{
			let j = JSON.stringify(eventName);
			jsi.setEventName(j);
		};

	ui.cbStartUpdate =
		function (length)
		{
			let progress = d.querySelector('progress.update');
			progress.max = length;
			progress.value = 0;
			progress.classList.add('show');
			d.querySelector('div.waitPage').classList.remove('show');
		};

	ui.cbProgressUpdate =
		function (index)
		{
			let progress = d.querySelector('progress.update');
			progress.value = parseInt(index) + 1;
		};

	d.addEventListener('DOMContentLoaded',
		function ()
		{
			setLang();
			onResize();
			w.addEventListener('resize', onResize);
			var def = d.body.getAttribute('default');
			let serialCode = document.querySelector('p.messageSerialCode');
			if (serialCode) serialCode.innerText = jsi.serialCode();
			if (d.body.classList.contains('mainActivity'))
			{
				if (w.jsi !== undefined)
				{
					ui.getButtons();
					ui.getEventName();
					getConfig(
						function ()
						{
							if (def) ui.onClickMenuItem(def);
						});
				}
				else
				{
					if (def) ui.onClickMenuItem(def);
				}
				d.addEventListener('contextmenu',
					function (e)
					{
						e.preventDefault();
					});
				d.addEventListener('click',
					function ()
					{
						ui.onContextMenuClose();
					});
				d.addEventListener('touchmove',
					function ()
					{
						ui.onContextMenuClose();
					});
			}
		});

w.ui = ui;})(window, document);
