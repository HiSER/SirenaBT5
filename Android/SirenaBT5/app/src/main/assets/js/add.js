'use strict';

DOMTokenList.prototype.clear =
	function ()
	{
		for (let item of this) this.remove(item);
	};

DOMTokenList.prototype.removeRegExp =
	function (regexp)
	{
		let m = new RegExp(regexp);
		for (let item of this)
		{
			if (item.match(m)) this.remove(item);
		}
	};