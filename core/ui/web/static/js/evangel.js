$(document).ready(
function() {

	function initialize_transfer() {
		[
			{ id: "#new-transfer", icon: "ui-icon-plus" },
			{ id: "#start-transfer", icon: "ui-icon-play" },
			{ id: "#pause-transfer", icon: "ui-icon-pause" },
			{ id: "#stop-transfer", icon: "ui-icon-stop" },
			{ id: "#remove-transfer", icon: "ui-icon-minus" }
		].forEach(function (info) {
			$(info.id).button({ icons: { primary: info.icon } });
		});

		// todo we need a little margin between the toolbar and the list
		$("#transfer-list").jqGrid({
			datatype: "local",
			autowidth: true,
			colNames: [ "Name", "Status", "Size", "Down Speed", "Up Speed" ],
			colModel: [
				{name: "name", index: "name"},
				{name: "status", index: "status"},
				{name: "size", index: "size"},
				{name: "downrate", index: "downrate"},
				{name: "uprate", index: "uprate"}
			]
		});
		$("#transfer-list").jqGrid("addRowData", 1, {name: "asdf", index: "xx"});
	}

	$("#main-tab").tabs();
	initialize_transfer();
}
)

